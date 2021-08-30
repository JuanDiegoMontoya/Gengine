#include "vPCH.h"
#include <voxel/ChunkRenderer.h>
#include <voxel/Chunk.h>

#include <engine/Camera.h>
#include <engine/gfx/Frustum.h>
#include <engine/gfx/ShaderManager.h>
#include <engine/gfx/TextureLoader.h>
#include <engine/gfx/DebugMarker.h>
#include <engine/gfx/Indirect.h>
#include <engine/gfx/Fence.h>
#include <engine/CVar.h>
#include <engine/gfx/DynamicBuffer.h>
#include <engine/Shapes.h>
#include <engine/core/Statistics.h>

#include <filesystem>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "ChunkRenderer.h"
#include <engine/core/StatMacros.h>

AutoCVar<cvar_float> cullDistanceMinCVar("v.cullDistanceMin", "- Minimum distance at which chunks should render", 0);
AutoCVar<cvar_float> cullDistanceMaxCVar("v.cullDistanceMax", "- Maximum distance at which chunks should render", 2000);
AutoCVar<cvar_float> freezeCullingCVar("v.freezeCulling", "- If enabled, freezes chunk culling", 0, 0, 1, CVarFlag::CHEAT);
AutoCVar<cvar_float> drawOcclusionVolumesCVar("v.drawOcclusionVolumes", "- If enabled, draws occlusion volumes", 0, 0, 1, CVarFlag::CHEAT);
AutoCVar<cvar_float> anisotropyCVar("v.anisotropy", "- Level of anisotropic filtering to apply to voxels", 16, 1, 16);

DECLARE_FLOAT_STAT(DrawVoxelsAll, GPU);
DECLARE_FLOAT_STAT(Joe, CPU);

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

    GLuint vao{};
    std::unique_ptr<GFX::StaticBuffer> dib;

    std::unique_ptr<GFX::StaticBuffer> drawCountGPU;

    // size of compute shader workgroup
    const int workGroupSize = 64; // defined in compact_batch.cs

    GLuint vaoCull{};
    std::unique_ptr<GFX::StaticBuffer> dibCull;
    GLsizei activeAllocs{};
    std::pair<uint64_t, GLuint> stateInfo{ 0, 0 };
    bool dirtyAlloc = true;
    std::unique_ptr<GFX::StaticBuffer> vertexAllocBuffer;

    // resources
    std::optional<GFX::Texture> blockTextures;
    std::optional<GFX::TextureView> blockTexturesView;
    std::optional<GFX::TextureSampler> blockTexturesSampler;
  };

  // call after all chunks are initialized
  ChunkRenderer::ChunkRenderer()
  {
    data = new ChunkRendererStorage;

    data->drawCountGPU = std::make_unique<GFX::StaticBuffer>(nullptr, sizeof(uint32_t));

    // allocate big buffers
    // TODO: vary the allocation size based on some user setting
    data->verticesAllocator = std::make_unique<GFX::DebugDrawableBuffer<AABB16>>(250'000'000, 2 * sizeof(uint32_t));

    /* :::::::::::BUFFER FORMAT:::::::::::
                            CHUNK 1                                    CHUNK 2                   NULL                   CHUNK 3
            | cpos, encoded+lighting, encoded+lighting, ... | cpos, encoded+lighting, ... | null (any length) | cpos, encoded+lighting, ... |
    First:   offset(CHUNK 1)=0                               offset(CHUNK 2)                                   offset(CHUNK 3)
    Draw commands will specify where in memory the draw call starts. This will account for variable offsets.
    
        :::::::::::BUFFER FORMAT:::::::::::*/
    glCreateVertexArrays(1, &data->vao);
    glEnableVertexArrayAttrib(data->vao, 0); // chunk position (one per instance)
    
    // stride is sizeof(vertex) so baseinstance can be set to cmd.first and work (hopefully)
    glVertexArrayAttribIFormat(data->vao, 0, 3, GL_INT, 0);

    glVertexArrayAttribBinding(data->vao, 0, 0);
    glVertexArrayBindingDivisor(data->vao, 0, 1);

    glVertexArrayVertexBuffer(data->vao, 0, data->verticesAllocator->GetID(), 0, 2 * sizeof(uint32_t));

    // setup vertex buffer for cube that will be used for culling
    glCreateVertexArrays(1, &data->vaoCull);

    DrawElementsIndirectCommand occlusionCullingCmd
    {
      .count = 14, // vertices on cube
      .instanceCount = 0, // will be incremented - reset every frame
      .firstIndex = 0,
      .baseVertex = 0,
      .baseInstance = 0
    };
    data->dibCull = std::make_unique<GFX::StaticBuffer>(&occlusionCullingCmd, sizeof(occlusionCullingCmd), GFX::BufferFlag::CLIENT_STORAGE);

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

    //engine::Core::StatisticsManager::Get()->RegisterFloatStat("DrawVoxelsAll", "GPU");
    engine::Core::StatisticsManager::Get()->RegisterFloatStat("DrawVisibleChunks", "GPU");
    engine::Core::StatisticsManager::Get()->RegisterFloatStat("GenerateDIB", "GPU");
    engine::Core::StatisticsManager::Get()->RegisterFloatStat("Stat4", "Test");
  }

  ChunkRenderer::~ChunkRenderer()
  {
    delete data;
  }

  void ChunkRenderer::DrawBuffers()
  {
    //glDisable(GL_DEPTH_TEST);

    auto sdr = GFX::ShaderManager::Get()->GetShader("buffer_vis");
    sdr->Bind();
    glm::mat4 model(1);
    model = glm::scale(model, { 1, 1, 1 });
    model = glm::translate(model, { -.5, -.90, 0 });
    sdr->SetMat4("u_model", model);

    glLineWidth(50);
    glDepthFunc(GL_ALWAYS);
    data->verticesAllocator->Draw();
    glLineWidth(2);

    //glEnable(GL_DEPTH_TEST);
  }

  void ChunkRenderer::Draw()
  {
    GFX::DebugMarker marker("Draw voxels");
    MEASURE_GPU_TIMER_STAT(DrawVoxelsAll);
    MEASURE_CPU_TIMER_STAT(Joe);

    engine::Core::StatisticsManager::Get()->PushFloatStatValue("Stat4", rand() % 3);

    RenderVisible();
    GenerateDIB();
    RenderOcclusion();
    //RenderRest();

    DrawBuffers();
  }

  void ChunkRenderer::RenderVisible()
  {
    // TODO: rendering is glitchy when modifying chunks rapidly
    // this is probably due to how the previous frame's visible chunks will be drawn
    GFX::DebugMarker marker("Draw visible chunks");
    MEASURE_GPU_TIMER_STAT(DrawVisibleChunks);

    if (!data->dib)
      return;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // don't forget to reset original culling face

    // render blocks in each active chunk
    auto currShader = GFX::ShaderManager::Get()->GetShader("chunk_optimized");
    currShader->Bind();

    //float angle = glm::max(glm::dot(-glm::normalize(NuRenderer::activeSun_->GetDir()), glm::vec3(0, 1, 0)), 0.f);
    static float u_minBrightness = 0.01f;
    static glm::vec3 u_envColor = glm::vec3(1);
    ImGui::SliderFloat("Min Brightness", &u_minBrightness, 0.0f, 0.1f);
    ImGui::SliderFloat3("Env Color", glm::value_ptr(u_envColor), 0.0f, 0.1f);
    currShader->SetFloat("u_minBrightness", u_minBrightness);
    currShader->SetVec3("u_envColor", u_envColor);

    // undo gamma correction for sky color
    static const glm::vec3 skyColor(
      glm::pow(.529f, 2.2f),
      glm::pow(.808f, 2.2f),
      glm::pow(.922f, 2.2f));
    currShader->SetMat4("u_viewProj", CameraSystem::GetProj() * CameraSystem::GetView());

    glBindVertexArray(data->vao);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data->verticesAllocator->GetID());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->verticesAllocator->GetID());
    data->dib->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
    data->drawCountGPU->Bind<GFX::Target::PARAMETER_BUFFER>();
    GFX::SamplerState state = data->blockTexturesSampler->GetState();
    state.asBitField.anisotropy = getAnisotropy(anisotropyCVar.Get());
    data->blockTexturesSampler->SetState(state);
    data->blockTexturesView->Bind(0, *data->blockTexturesSampler);
    glMultiDrawArraysIndirectCount(GL_TRIANGLES, 0, 0, data->activeAllocs, 0);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    data->blockTexturesView->Unbind(0);
  }

  void ChunkRenderer::GenerateDIB()
  {
    GFX::DebugMarker marker("Generate draw commands");
    MEASURE_GPU_TIMER_STAT(GenerateDIB);

    if (freezeCullingCVar.Get())
      return;

    auto sdr = GFX::ShaderManager::Get()->GetShader("compact_batch");
    sdr->Bind();

    // set uniforms for chunk rendering
    sdr->SetVec3("u_viewpos", CameraSystem::GetPos());
    Frustum fr = *CameraSystem::GetFrustum();
    for (int i = 0; i < 5; i++) // ignore near plane
    {
      std::string uname = "u_viewfrustum.data_[" + std::to_string(i) + "][0]";
      sdr->Set1FloatArray(hashed_string(uname.c_str()), std::span<float, 4>(fr.GetData()[i]));
    }
    sdr->SetFloat("u_cullMinDist", cullDistanceMinCVar.Get());
    sdr->SetFloat("u_cullMaxDist", cullDistanceMaxCVar.Get());
    sdr->SetUInt("u_reservedBytes", 16);
    sdr->SetUInt("u_quadSize", sizeof(uint32_t) * 2);

    constexpr uint32_t zero = 0;
    data->drawCountGPU->SubData(&zero, sizeof(uint32_t));

    const auto& vertexAllocs = data->verticesAllocator->GetAllocs();

    // only re-construct if allocator has been modified
    if (data->dirtyAlloc)
    {
      data->vertexAllocBuffer = std::make_unique<GFX::StaticBuffer>(vertexAllocs.data(), data->verticesAllocator->AllocSize() * vertexAllocs.size());
      data->dib = std::make_unique<GFX::StaticBuffer>(nullptr, data->verticesAllocator->ActiveAllocs() * sizeof(DrawArraysIndirectCommand));
      data->dirtyAlloc = false;
      data->verticesAllocator->GenDrawData();
    }

    data->vertexAllocBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
    data->dib->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
    data->drawCountGPU->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(2);

    int numWorkGroups = (vertexAllocs.size() + data->workGroupSize - 1) / data->workGroupSize;
    glDispatchCompute(numWorkGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    data->activeAllocs = data->verticesAllocator->ActiveAllocs();
  }

  void ChunkRenderer::RenderOcclusion()
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

    //const glm::mat4 viewProj = cam->GetProj() * cam->GetView();
    //Camera* cam = Camera::ActiveCamera;
    const glm::mat4 viewProj = CameraSystem::GetProj() * CameraSystem::GetView();
    sr->SetMat4("u_viewProj", viewProj);
    sr->SetUInt("u_chunk_size", Chunk::CHUNK_SIZE);
    sr->SetBool("u_debugDraw", drawOcclusionVolumesCVar.Get() != 0.0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data->verticesAllocator->GetID());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->verticesAllocator->GetID());

    data->dib->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);

    // copy # of chunks being drawn (parameter buffer) to instance count (DIB)
    data->dibCull->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
    glBindVertexArray(data->vaoCull);
    constexpr GLint offset = offsetof(DrawArraysIndirectCommand, instanceCount);
    glCopyNamedBufferSubData(data->drawCountGPU->GetID(), data->dibCull->GetID(), 0, offset, sizeof(uint32_t));
    //glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, (void*)0, 1, 0);
    glDrawArraysIndirect(GL_TRIANGLE_STRIP, 0);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glEnable(GL_CULL_FACE);
    glDepthMask(true);
    glColorMask(true, true, true, true);
  }

  void ChunkRenderer::RenderRest()
  {
    GFX::DebugMarker marker("Draw disoccluded chunks");
    // Drawing logic:
    // for each Chunk in Chunks
    //   if Chunk was not rendered in RenderVisible and not occluded
    //     draw(Chunk)

    // resources:
    // DIB with draw info

    // IDEA: RenderOcclusion generates a mask to use for the NEXT frame's RenderRest pass
    // the mask will contain all the chunks that were to be drawn at the start of that frame's RenderVisible pass
    // the current frame will 

    ASSERT(0); // not implemented
  }

  uint64_t ChunkRenderer::AllocChunk(std::span<uint32_t> vertices, const AABB& aabb)
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

  void ChunkRenderer::FreeChunk(uint64_t allocHandle)
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