#include "vPCH.h"
#include "Chunk.h"
#include "ChunkRenderer.h"

#include <engine/gfx/Frustum.h>
#include <engine/gfx/ShaderManager.h>
#include <engine/gfx/TextureLoader.h>
#include <engine/gfx/DebugMarker.h>
#include <engine/gfx/Indirect.h>
#include <engine/gfx/Fence.h>
#include <engine/gfx/DynamicBuffer.h>
#include <engine/gfx/Camera.h>
#include <engine/gfx/Framebuffer.h>
#include <engine/gfx/RenderView.h>
#include <engine/gfx/TextureManager.h>
#include <engine/gfx/Renderer.h>

#include <engine/CVar.h>
#include <engine/Shapes.h>
#include <engine/core/Statistics.h>

#include <filesystem>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <engine/core/StatMacros.h>

AutoCVar<cvar_float> cullDistanceMinCVar("v.cullDistanceMin", "- Minimum distance at which chunks should render", 0);
AutoCVar<cvar_float> cullDistanceMaxCVar("v.cullDistanceMax", "- Maximum distance at which chunks should render", 2000);
AutoCVar<cvar_float> freezeCullingCVar("v.freezeCulling", "- If enabled, freezes chunk culling", 0, 0, 1, CVarFlag::CHEAT);
AutoCVar<cvar_float> drawOcclusionVolumesCVar("v.drawOcclusionVolumes", "- If enabled, draws occlusion volumes", 0, 0, 1, CVarFlag::CHEAT);
AutoCVar<cvar_float> anisotropyCVar("v.anisotropy", "- Level of anisotropic filtering to apply to voxels", 16, 1, 16);
AutoCVar<cvar_float> lowQualityCullDistance("v.lowQualityCullDistance", "- Maximum distance at which chunks for low quality cameras should render", 100);

DECLARE_FLOAT_STAT(DrawVoxelsAll, GPU)

static GFX::Anisotropy getAnisotropy(cvar_float val)
{
  if (val >= 16) return GFX::Anisotropy::SAMPLES_16;
  else if (val >= 8) return GFX::Anisotropy::SAMPLES_8;
  else if (val >= 4) return GFX::Anisotropy::SAMPLES_4;
  else if (val >= 2) return GFX::Anisotropy::SAMPLES_2;
  return GFX::Anisotropy::SAMPLES_1;
}

namespace Voxels
{
  struct ChunkRendererStorage
  {
    std::unique_ptr<GFX::DebugDrawableBuffer<AABB16>> verticesAllocator;
    std::unordered_set<uint64_t> vertexAllocHandles;

    GLuint chunkVao{};

    struct ViewData
    {
      std::unique_ptr<GFX::StaticBuffer> drawIndirectBuffer;
      std::unique_ptr<GFX::StaticBuffer> drawCountParameterBuffer;
    };

    std::unordered_map<GFX::RenderView, ViewData, GFX::hash<GFX::RenderView>> perViewData;

    // size of compute shader workgroup
    const int workGroupSize = 64; // defined in compact_batch.cs

    GLuint occlusionVao{};
    std::unique_ptr<GFX::StaticBuffer> occlusionDib;
    GLsizei activeAllocs{};
    //std::pair<uint64_t, GLuint> stateInfo{ 0, 0 };
    bool dirtyAlloc = true;
    std::unique_ptr<GFX::StaticBuffer> vertexAllocBuffer;

    // resources
    std::optional<GFX::Texture> blockTextures;
    std::optional<GFX::TextureView> blockTexturesView;
    std::optional<GFX::TextureSampler> blockTexturesSampler;
    std::optional<GFX::TextureSampler> probeSampler;
  };

  // call after all chunks are initialized
  ChunkRenderer::ChunkRenderer()
  {
    data = new ChunkRendererStorage;

    // allocate big buffers
    // TODO: vary the allocation size based on some user setting
    data->verticesAllocator = std::make_unique<GFX::DebugDrawableBuffer<AABB16>>(1'000'000, 2 * sizeof(uint32_t));

    /* :::::::::::BUFFER FORMAT:::::::::::
                            CHUNK 1                                    CHUNK 2                   NULL                   CHUNK 3
            | cpos, encoded+lighting, encoded+lighting, ... | cpos, encoded+lighting, ... | null (any length) | cpos, encoded+lighting, ... |
    First:   offset(CHUNK 1)=0                               offset(CHUNK 2)                                   offset(CHUNK 3)
    Draw commands will specify where in memory the draw call starts. This will account for variable offsets.
    
        :::::::::::BUFFER FORMAT:::::::::::*/
    glCreateVertexArrays(1, &data->chunkVao);
    glEnableVertexArrayAttrib(data->chunkVao, 0); // chunk position (one per instance)
    
    // stride is sizeof(vertex) so baseinstance can be set to cmd.first and work (hopefully)
    glVertexArrayAttribIFormat(data->chunkVao, 0, 3, GL_INT, 0);

    glVertexArrayAttribBinding(data->chunkVao, 0, 0);
    glVertexArrayBindingDivisor(data->chunkVao, 0, 1);

    glVertexArrayVertexBuffer(data->chunkVao, 0, data->verticesAllocator->GetID(), 0, 2 * sizeof(uint32_t));

    // setup vertex buffer for cube that will be used for culling
    glCreateVertexArrays(1, &data->occlusionVao);

    DrawElementsIndirectCommand occlusionCullingCmd
    {
      .count = 14, // vertices on cube
      .instanceCount = 0, // will be incremented - reset every frame
      .firstIndex = 0,
      .baseVertex = 0,
      .baseInstance = 0
    };
    data->occlusionDib = std::make_unique<GFX::StaticBuffer>(&occlusionCullingCmd, sizeof(occlusionCullingCmd), GFX::BufferFlag::CLIENT_STORAGE);

    // assets
    std::vector<std::string> texs;
    for (const auto& prop : Block::PropertiesTable)
    {
      std::string path = std::string(prop.name) + ".png";
      std::string realPath = std::string(TextureDir) + std::string(path);
      bool hasTex = std::filesystem::exists(realPath);
      if (!hasTex)
      {
        spdlog::warn("Texture {} does not exist, using fallback.", path);
        path = "error.png";
      }
      texs.push_back(path);
    }
    std::vector<std::string_view> texsView(texs.begin(), texs.end());
    data->blockTextures = GFX::LoadTexture2DArray(texsView);

    data->blockTexturesView = GFX::TextureView::Create(*data->blockTextures);

    GFX::SamplerState ss;
    ss.asBitField.magFilter = GFX::Filter::NEAREST;
    ss.asBitField.minFilter = GFX::Filter::LINEAR;
    ss.asBitField.mipmapFilter = GFX::Filter::LINEAR;
    ss.asBitField.anisotropy = GFX::Anisotropy::SAMPLES_16;
    data->blockTexturesSampler = GFX::TextureSampler::Create(ss);

    GFX::SamplerState cubeSs{};
    cubeSs.asBitField.magFilter = GFX::Filter::LINEAR;
    cubeSs.asBitField.minFilter = GFX::Filter::LINEAR;
    cubeSs.asBitField.addressModeU = GFX::AddressMode::CLAMP_TO_EDGE;
    cubeSs.asBitField.addressModeV = GFX::AddressMode::CLAMP_TO_EDGE;
    cubeSs.asBitField.addressModeW = GFX::AddressMode::CLAMP_TO_EDGE;
    data->probeSampler = GFX::TextureSampler::Create(cubeSs);

    //engine::Core::StatisticsManager::Get()->RegisterFloatStat("DrawVoxelsAll", "GPU");
    engine::Core::StatisticsManager::Get()->RegisterFloatStat("DrawVisibleChunks", "GPU");
    engine::Core::StatisticsManager::Get()->RegisterFloatStat("GenerateDIB", "GPU");
  }

  ChunkRenderer::~ChunkRenderer()
  {
    delete data;
  }

  void ChunkRenderer::DrawBuffers(std::span<GFX::RenderView> renderViews)
  {
    auto sdr = GFX::ShaderManager::Get()->GetShader("buffer_vis");
    sdr->Bind();
    glm::mat4 model(1);
    model = glm::scale(model, { 1, 1, 1 });
    model = glm::translate(model, { -.5, -.90, 0 });
    sdr->SetMat4("u_model", model);

    glLineWidth(50);
    glDepthFunc(GL_ALWAYS);

    for (auto& renderView : renderViews)
    {
      if (!(renderView.mask & GFX::RenderMaskBit::RenderScreenElements))
        continue;

      glViewport(renderView.offset.x, renderView.offset.y, renderView.size.width, renderView.size.height);
      renderView.renderTarget->Bind();

      renderView.renderTarget->Bind();
      data->verticesAllocator->Draw();
    }

    glLineWidth(2);
  }

  void ChunkRenderer::Draw(std::span<GFX::RenderView> renderViews)
  {
    GFX::DebugMarker marker("Draw voxels");
    MEASURE_GPU_TIMER_STAT(DrawVoxelsAll);

    // init data for newly-created cameras
    for (auto& renderView : renderViews)
    {
      if (!(renderView.mask & GFX::RenderMaskBit::RenderVoxels))
        continue;

      if (!data->perViewData.contains(renderView))
      {
        data->perViewData[renderView].drawCountParameterBuffer = std::make_unique<GFX::StaticBuffer>(nullptr, sizeof(uint32_t));
      }
    }

    RenderVisibleChunks(renderViews);
    GenerateDrawIndirectBuffer(renderViews);
    RenderOcclusion(renderViews);
    //RenderDisoccludedThisFrame(renderViews);
  }

  void ChunkRenderer::RenderVisibleChunks(std::span<GFX::RenderView> renderViews)
  {
    // TODO: rendering is glitchy when modifying chunks rapidly
    // this is probably due to how the previous frame's visible chunks will be drawn
    GFX::DebugMarker marker("Draw visible chunks");
    MEASURE_GPU_TIMER_STAT(DrawVisibleChunks);

    static float u_minBrightness = 0.01f;
    static glm::vec3 u_envColor = glm::vec3(1);
    ImGui::SliderFloat("Min Brightness", &u_minBrightness, 0.0f, 0.1f);
    ImGui::SliderFloat3("Env Color", glm::value_ptr(u_envColor), 0.0f, 0.1f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // don't forget to reset original culling face
    glBindVertexArray(data->chunkVao);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data->verticesAllocator->GetID());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->verticesAllocator->GetID());

    // render blocks in each active chunk
    auto currShader = GFX::ShaderManager::Get()->GetShader("chunk_optimized");
    currShader->Bind();

    currShader->SetFloat("u_minBrightness", u_minBrightness);
    currShader->SetVec3("u_envColor", u_envColor);
    GFX::SamplerState state = data->blockTexturesSampler->GetState();
    state.asBitField.anisotropy = getAnisotropy(anisotropyCVar.Get());
    data->blockTexturesSampler->SetState(state);
    data->blockTexturesView->Bind(0, *data->blockTexturesSampler);
    auto probeView = GFX::TextureView::Create(*GFX::TextureManager::Get()->GetTexture("probeColor"));
    probeView->Bind(1, *data->probeSampler);

    auto bufs1 = { GFX::Attachment::COLOR_0, GFX::Attachment::COLOR_1 };
    GFX::Renderer::Get()->GetMainFramebuffer()->SetDrawBuffers(bufs1);

    for (auto& renderView : renderViews)
    {
      if (!(renderView.mask & GFX::RenderMaskBit::RenderVoxels))
        continue;
      
      auto& [drawIndirectBuffer, parameterBuffer] = data->perViewData[renderView];
      if (!drawIndirectBuffer)
        continue;

      glViewport(renderView.offset.x, renderView.offset.y, renderView.size.width, renderView.size.height);
      renderView.renderTarget->Bind();

      currShader->SetBool("u_disableOcclusionCulling", !!(renderView.mask & GFX::RenderMaskBit::RenderVoxelsNear));
      currShader->SetVec3("u_viewpos", renderView.camera->viewInfo.position);
      currShader->SetMat4("u_viewProj", renderView.camera->GetViewProj());

      drawIndirectBuffer->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
      parameterBuffer->Bind<GFX::Target::PARAMETER_BUFFER>();
      glMultiDrawArraysIndirectCount(GL_TRIANGLES, 0, 0, data->activeAllocs, 0);
      glTextureBarrier();
    }

    auto bufs2 = { GFX::Attachment::COLOR_0, GFX::Attachment::NONE };
    GFX::Renderer::Get()->GetMainFramebuffer()->SetDrawBuffers(bufs2);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    probeView->Unbind(0);
    data->blockTexturesView->Unbind(0);
  }

  void ChunkRenderer::GenerateDrawIndirectBuffer(std::span<GFX::RenderView> renderViews)
  {
    GFX::DebugMarker marker("Generate draw commands");
    MEASURE_GPU_TIMER_STAT(GenerateDIB);

    if (freezeCullingCVar.Get())
      return;

    auto sdr = GFX::ShaderManager::Get()->GetShader("compact_batch");
    sdr->Bind();
    sdr->SetFloat("u_cullMinDist", cullDistanceMinCVar.Get());
    sdr->SetFloat("u_cullMaxDist", cullDistanceMaxCVar.Get());
    sdr->SetUInt("u_reservedBytes", 16);
    sdr->SetUInt("u_quadSize", sizeof(uint32_t) * 2);
    const auto& vertexAllocs = data->verticesAllocator->GetAllocs();
    uint32_t numWorkGroups = (vertexAllocs.size() + data->workGroupSize - 1) / data->workGroupSize;

    if (data->dirtyAlloc)
    {
      data->vertexAllocBuffer = std::make_unique<GFX::StaticBuffer>(vertexAllocs.data(), data->verticesAllocator->AllocSize() * vertexAllocs.size());
      data->verticesAllocator->GenDrawData();
    }

    data->vertexAllocBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);

    for (auto& renderView : renderViews)
    {
      if (!(renderView.mask & GFX::RenderMaskBit::RenderVoxels))
        continue;

      if (renderView.mask & GFX::RenderMaskBit::RenderVoxelsNear)
      {
        sdr->SetBool("u_disableOcclusionCulling", true);
        sdr->SetFloat("u_lowQualityCullDistance", lowQualityCullDistance.Get());
      }
      else
      {
        sdr->SetBool("u_disableOcclusionCulling", false);
      }

      auto& [drawIndirectBuffer, parameterBuffer] = data->perViewData[renderView];

      // set uniforms for chunk rendering
      sdr->SetVec3("u_viewpos", renderView.camera->viewInfo.position);
      GFX::Frustum fr(renderView.camera->proj, renderView.camera->viewInfo.GetViewMatrix());
      for (int i = 0; i < 5; i++) // ignore near plane
      {
        std::string uname = "u_viewfrustum.data_[" + std::to_string(i) + "][0]";
        sdr->Set1FloatArray(hashed_string(uname.c_str()), std::span<float, 4>(fr.GetData()[i]));
      }

      constexpr uint32_t zero = 0;
      parameterBuffer->SubData(&zero, sizeof(uint32_t));

      // only re-construct if allocator has been modified
      if (data->dirtyAlloc)
      {
        drawIndirectBuffer = std::make_unique<GFX::StaticBuffer>(nullptr, data->verticesAllocator->ActiveAllocs() * sizeof(DrawArraysIndirectCommand));
      }

      drawIndirectBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
      parameterBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(2);

      glDispatchCompute(numWorkGroups, 1, 1);
    }

    if (data->dirtyAlloc)
    {
      data->dirtyAlloc = false;
    }

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    data->activeAllocs = data->verticesAllocator->ActiveAllocs();
  }

  void ChunkRenderer::RenderOcclusion(std::span<GFX::RenderView> renderViews)
  {
    GFX::DebugMarker marker("Draw occlusion volumes");
    if (freezeCullingCVar.Get())
      return;

    if (drawOcclusionVolumesCVar.Get() == 0.0)
    {
      glColorMask(false, false, false, false); // false = can't be written
      glDepthMask(false);
    }
    glDisable(GL_CULL_FACE);

    auto sr = GFX::ShaderManager::Get()->GetShader("chunk_render_cull");
    sr->Bind();
    sr->SetUInt("u_chunk_size", Chunk::CHUNK_SIZE);
    sr->SetBool("u_debugDraw", drawOcclusionVolumesCVar.Get() != 0.0);

    for (auto& renderView : renderViews)
    {
      // only draw occlusion for views that draw voxels AND don't have the RenderVoxelsNear bit set
      if ((renderView.mask & GFX::RenderMaskBit::RenderVoxelsNear) ||
        !(renderView.mask & GFX::RenderMaskBit::RenderVoxels))
      {
        continue;
      }

      auto& [drawIndirectBuffer, parameterBuffer] = data->perViewData[renderView];

      glViewport(renderView.offset.x, renderView.offset.y, renderView.size.width, renderView.size.height);

      renderView.renderTarget->Bind();

      const glm::mat4 viewProj = renderView.camera->GetViewProj();
      sr->SetMat4("u_viewProj", viewProj);

      glBindBuffer(GL_SHADER_STORAGE_BUFFER, data->verticesAllocator->GetID());
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->verticesAllocator->GetID());

      drawIndirectBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);

      // copy # of chunks being drawn (parameter buffer) to instance count (DIB)
      data->occlusionDib->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
      glBindVertexArray(data->occlusionVao);
      constexpr GLint offset = offsetof(DrawArraysIndirectCommand, instanceCount);
      glCopyNamedBufferSubData(parameterBuffer->GetID(), data->occlusionDib->GetID(), 0, offset, sizeof(uint32_t));
      glDrawArraysIndirect(GL_TRIANGLE_STRIP, 0);
    }

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glEnable(GL_CULL_FACE);
    glDepthMask(true);
    glColorMask(true, true, true, true);
  }

  void ChunkRenderer::RenderDisoccludedThisFrame([[maybe_unused]] std::span<GFX::RenderView> renderViews)
  {
    GFX::DebugMarker marker("Draw disoccluded chunks");
    // Drawing logic:
    // for each Chunk in Chunks
    //   if Chunk was not rendered in RenderVisibleChunks and not occluded
    //     draw(Chunk)

    // resources:
    // DIB with draw info

    // IDEA: RenderOcclusion generates a mask to use for the NEXT frame's RenderDisoccludedThisFrame pass
    // the mask will contain all the chunks that were to be drawn at the start of that frame's RenderVisibleChunks pass
    // the current frame will ...
    
    ASSERT(0); // not implemented
  }

  uint64_t ChunkRenderer::AllocChunkMesh(std::span<uint32_t> vertices, const AABB& aabb)
  {
    uint64_t vertexBufferHandle{};

    // free oldest allocations until there is enough space to allocate this buffer
    //while ((vertexBufferHandle = data->verticesAllocator->Allocate(
    //  vertices.data(),
    //  vertices.size() * sizeof(GLint),
    //  aabb)) == 0)
    //{
    //  data->verticesAllocator->FreeOldest();
    //}
    vertexBufferHandle = data->verticesAllocator->Allocate(vertices.data(), vertices.size() * sizeof(GLint), aabb);
    if (!vertexBufferHandle)
    {
      data->verticesAllocator->Free(vertexBufferHandle);
      return 0;
    }

    data->vertexAllocHandles.emplace(vertexBufferHandle);
    data->dirtyAlloc = true;

    return vertexBufferHandle;
  }

  void ChunkRenderer::FreeChunkMesh(uint64_t allocHandle)
  {
    auto it = data->vertexAllocHandles.find(allocHandle);
    if (it == data->vertexAllocHandles.end())
      return;
    uint64_t handle = *it;
    data->verticesAllocator->Free(handle);
    data->vertexAllocHandles.erase(it);
    data->dirtyAlloc = true;
  }
}


/*
* Idea:
*   per-face data: normal (0 bits), texture ID (14 bits?), position [0, 32] (18 bits), lighting (16 bits)
*   per-vertex data: ambient occlusion (2 bits)
*
* Strategy:
*   SSBO face data:
*     u32: position (18), textureID (10 bits)           -> 4 unused bits
*     u32: lighting (16), AO (for all four vertices, 8) -> 8 unused bits
*
*   "code":
*     reconstruct vertex position on quad by gl_VertexID
*     reconstruct normal in fragment shader with partial derivatives
*
*   Result will be 8 bytes per quad. Upside: easy anisotropic ambient occlusion (bilinear filter).
*   No index buffer necessary.
*/