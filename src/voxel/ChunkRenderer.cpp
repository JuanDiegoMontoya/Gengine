#include "vPCH.h"
#include <voxel/ChunkRenderer.h>
#include <engine/gfx/DynamicBuffer.h>
#include <voxel/Chunk.h>
#include <engine/Camera.h>
#include <engine/gfx/Frustum.h>
#include <execution>
#include <engine/gfx/ShaderManager.h>
#include <engine/gfx/Vertices.h>
#include <engine/gfx/MeshUtils.h>
#include <engine/gfx/TextureLoader.h>
#include <engine/gfx/DebugMarker.h>
#include <engine/gfx/Indirect.h>
#include <engine/CVar.h>
#include <engine/gfx/DynamicBuffer.h>

#include <filesystem>

#include <imgui/imgui.h>
#include "ChunkRenderer.h"

AutoCVar<cvar_float> cullDistanceMinCVar("v.cullDistanceMin", "- Minimum distance at which chunks should render", 0);
AutoCVar<cvar_float> cullDistanceMaxCVar("v.cullDistanceMax", "- Maximum distance at which chunks should render", 2000);
AutoCVar<cvar_float> freezeCullingCVar("v.freezeCulling", "- If enabled, freezes chunk culling", 0, 0, 1, CVarFlag::CHEAT);
AutoCVar<cvar_float> drawOcclusionVolumesCVar("v.drawOcclusionVolumes", "- If enabled, draws occlusion volumes", 0, 0, 1, CVarFlag::CHEAT);
AutoCVar<cvar_float> anisotropyCVar("v.anisotropy", "- Level of anisotropic filtering to apply to voxels", 16, 1, 16);

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
    std::unique_ptr<GFX::DynamicBuffer<AABB16>> verticesAllocator;
    std::unique_ptr<GFX::DynamicBuffer<>> indicesAllocator;
    std::unordered_map<uint64_t, glm::u64vec2> allocHandles;
    uint64_t currentAllocHandle{ 1 };

    GLuint vao{};
    std::unique_ptr<GFX::StaticBuffer> dib;

    std::unique_ptr<GFX::StaticBuffer> drawCountGPU;

    // size of compute block  for the compute shader
    const int groupSize = 64; // defined in compact_batch.cs

    GLuint vaoCull{};
    std::unique_ptr<GFX::StaticBuffer> dibCull;
    GLsizei activeAllocs{};
    std::pair<uint64_t, GLuint> stateInfo{ 0, 0 };
    bool dirtyAlloc = true;
    std::unique_ptr<GFX::StaticBuffer> vertexAllocBuffer;
    std::unique_ptr<GFX::StaticBuffer> indexAllocBuffer;
    std::unique_ptr<GFX::StaticBuffer> allocsIndices; // holds list of indices that point into the above two buffers

    // resources
    std::optional<GFX::Texture> blockTextures;
    std::optional<GFX::TextureView> blockTexturesView;
    std::optional<GFX::TextureSampler> blockTexturesSampler;

    std::optional<GFX::Texture> blueNoiseTexture;
    std::optional<GFX::TextureView> blueNoiseView;
    std::optional<GFX::TextureSampler> blueNoiseSampler;
  };

  // call after all chunks are initialized
  ChunkRenderer::ChunkRenderer()
  {
    data = new ChunkRendererStorage;

    data->drawCountGPU = std::make_unique<GFX::StaticBuffer>(nullptr, sizeof(GLint));

    // allocate big buffers
    // TODO: vary the allocation size based on some user setting
    data->verticesAllocator = std::make_unique<GFX::DynamicBuffer<AABB16>>(100'000'000, 2 * sizeof(GLint));
    data->indicesAllocator = std::make_unique<GFX::DynamicBuffer<>>(100'000'000, sizeof(uint32_t));

    /* :::::::::::BUFFER FORMAT:::::::::::
                            CHUNK 1                                    CHUNK 2                   NULL                   CHUNK 3
            | cpos, encoded+lighting, encoded+lighting, ... | cpos, encoded+lighting, ... | null (any length) | cpos, encoded+lighting, ... |
    First:   offset(CHUNK 1)=0                               offset(CHUNK 2)                                   offset(CHUNK 3)
    Draw commands will specify where in memory the draw call starts. This will account for variable offsets.
    
        :::::::::::BUFFER FORMAT:::::::::::*/
    glCreateVertexArrays(1, &data->vao);
    glEnableVertexArrayAttrib(data->vao, 0); // lighting
    glEnableVertexArrayAttrib(data->vao, 1); // encoded data
    glEnableVertexArrayAttrib(data->vao, 2); // chunk position (one per instance)
    
    // stride is sizeof(vertex) so baseinstance can be set to cmd.first and work (hopefully)
    glVertexArrayAttribIFormat(data->vao, 2, 3, GL_INT, 0);

    // move forward by TWO vertex sizes (vertex aligned)
    glVertexArrayAttribIFormat(data->vao, 0, 1, GL_UNSIGNED_INT, 4 * sizeof(uint32_t));
    
    glVertexArrayAttribIFormat(data->vao, 1, 1, GL_UNSIGNED_INT, 5 * sizeof(uint32_t));

    glVertexArrayAttribBinding(data->vao, 0, 0);
    glVertexArrayAttribBinding(data->vao, 1, 0);
    glVertexArrayAttribBinding(data->vao, 2, 1);
    glVertexArrayBindingDivisor(data->vao, 1, 1);

    glVertexArrayVertexBuffer(data->vao, 0, data->verticesAllocator->GetGPUHandle(), 0, 2 * sizeof(uint32_t));
    glVertexArrayVertexBuffer(data->vao, 1, data->verticesAllocator->GetGPUHandle(), 0, 2 * sizeof(uint32_t));
    glVertexArrayElementBuffer(data->vao, data->indicesAllocator->GetGPUHandle());

    // setup vertex buffer for cube that will be used for culling
    glCreateVertexArrays(1, &data->vaoCull);

    DrawElementsIndirectCommand cmd;
    cmd.count = 14; // vertices on cube
    cmd.instanceCount = 0; // will be incremented - reset every frame
    cmd.baseVertex = 0;
    cmd.firstIndex = 0;
    cmd.baseInstance = 0;
    data->dibCull = std::make_unique<GFX::StaticBuffer>(&cmd, sizeof(cmd), GFX::BufferFlag::CLIENT_STORAGE);

    // assets
    std::vector<std::string> texs;
    for (const auto& prop : Block::PropertiesTable)
    {
      std::string path = std::string(prop.name) + ".png";
      std::string realPath = std::string(TextureDir) + std::string(path);
      bool hasTex = std::filesystem::exists(realPath);
      if (!hasTex)
      {
        printf("Texture %s does not exist, using fallback.\n", path.c_str());
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

    data->blueNoiseTexture = GFX::LoadTexture2D("BlueNoise/64_64/LDR_LLL1_0.png", GFX::Format::R8G8B8A8_UNORM);
    data->blueNoiseView = GFX::TextureView::Create(*data->blueNoiseTexture);
    data->blueNoiseSampler = GFX::TextureSampler::Create(GFX::SamplerState{});
  }

  ChunkRenderer::~ChunkRenderer()
  {
    delete data;
  }

  void ChunkRenderer::DrawBuffers()
  {
    glDisable(GL_DEPTH_TEST);

    auto sdr = GFX::ShaderManager::Get()->GetShader("buffer_vis");
    sdr->Bind();
    glm::mat4 model(1);
    model = glm::scale(model, { 1, 1, 1 });
    model = glm::translate(model, { -.5, -.90, 0 });
    sdr->SetMat4("u_model", model);

    glLineWidth(50);
    //allocator->Draw();
    glLineWidth(2);

    glEnable(GL_DEPTH_TEST);
  }

  void ChunkRenderer::Draw()
  {
    GFX::DebugMarker marker("Draw voxels");

    RenderVisible();
    GenerateDIB();
    RenderOcclusion();
    //RenderRest();
  }

  void ChunkRenderer::RenderVisible()
  {
    // TODO: rendering is glitchy when modifying chunks rapidly
    // this is probably due to how the previous frame's visible chunks will be drawn
    GFX::DebugMarker marker("Draw visible chunks");
    if (!data->dib)
      return;

#ifdef TRACY_ENABLE
    TracyGpuZone("Normal Render");
#endif

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // don't forget to reset original culling face

    // render blocks in each active chunk
    auto currShader = GFX::ShaderManager::Get()->GetShader("chunk_optimized");
    currShader->Bind();

    //float angle = glm::max(glm::dot(-glm::normalize(NuRenderer::activeSun_->GetDir()), glm::vec3(0, 1, 0)), 0.f);
    static float angle = 2.0f;
    ImGui::SliderFloat("Sunlight strength", &angle, 0.f, 5.f);
    currShader->SetFloat("sunAngle", angle);

    // undo gamma correction for sky color
    static const glm::vec3 skyColor(
      glm::pow(.529f, 2.2f),
      glm::pow(.808f, 2.2f),
      glm::pow(.922f, 2.2f));
    currShader->SetVec3("viewPos", CameraSystem::GetPos());
    currShader->SetFloat("fogStart", 400.0f);
    currShader->SetFloat("fogEnd", 2000.0f);
    currShader->SetVec3("fogColor", skyColor);
    currShader->SetMat4("u_viewProj", CameraSystem::GetProj() * CameraSystem::GetView());

    glBindVertexArray(data->vao);
    data->dib->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
    data->drawCountGPU->Bind<GFX::Target::PARAMETER_BUFFER>();
    auto state = data->blockTexturesSampler->GetState();
    state.asBitField.anisotropy = getAnisotropy(anisotropyCVar.Get());
    data->blockTexturesSampler->SetState(state);
    data->blockTexturesView->Bind(0, *data->blockTexturesSampler);
    data->blueNoiseView->Bind(1, *data->blueNoiseSampler);
    //glMultiDrawArraysIndirectCount(GL_TRIANGLES, (void*)0, (GLintptr)0, activeAllocs, 0);
    glMultiDrawElementsIndirectCount(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, data->activeAllocs, 0);
    data->blueNoiseView->Unbind(1);
    data->blockTexturesView->Unbind(0);
  }

  void ChunkRenderer::GenerateDIB()
  {
    if (freezeCullingCVar.Get())
      return;

    GFX::DebugMarker marker("Generate draw commands");
#ifdef TRACY_ENABLE
    TracyGpuZone("Gen draw commands norm");
#endif

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
    sdr->SetUInt("u_reservedVertices", 2);
    sdr->SetUInt("u_vertexSize", sizeof(uint32_t) * 2);
    sdr->SetUInt("u_indexSize", sizeof(uint32_t));

    constexpr uint32_t zero = 0;
    data->drawCountGPU->SubData(&zero, sizeof(uint32_t));

    const auto& vertexAllocs = data->verticesAllocator->GetAllocs();
    const auto& indexAllocs = data->indicesAllocator->GetAllocs();

    // only re-construct if allocator has been modified
    if (data->dirtyAlloc)
    {
      data->vertexAllocBuffer = std::make_unique<GFX::StaticBuffer>(vertexAllocs.data(), data->verticesAllocator->AllocSize() * vertexAllocs.size());
      data->indexAllocBuffer = std::make_unique<GFX::StaticBuffer>(indexAllocs.data(), data->indicesAllocator->AllocSize() * indexAllocs.size());
      auto indicesVec = GetAllocIndices();
      data->allocsIndices = std::make_unique<GFX::StaticBuffer>(indicesVec.data(), indicesVec.size() * sizeof(glm::uvec2));
      data->dib = std::make_unique<GFX::StaticBuffer>(nullptr, data->verticesAllocator->ActiveAllocs() * sizeof(DrawElementsIndirectCommand));
      data->dirtyAlloc = false;
    }

    data->vertexAllocBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
    data->indexAllocBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
    data->allocsIndices->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(2);
    data->dib->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(3);
    data->drawCountGPU->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(4);

    {
      int numBlocks = (vertexAllocs.size() + data->groupSize - 1) / data->groupSize;
      glDispatchCompute(numBlocks, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // make SSBO writes visible to subsequent execution
    }

    data->activeAllocs = data->verticesAllocator->ActiveAllocs();
  }

  void ChunkRenderer::RenderOcclusion()
  {
    GFX::DebugMarker marker("Draw occlusion volumes");
    if (freezeCullingCVar.Get())
      return;

#ifdef TRACY_ENABLE
    TracyGpuZone("Occlusion Render");
#endif

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

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data->verticesAllocator->GetGPUHandle());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->verticesAllocator->GetGPUHandle());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data->dib->GetID());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, data->dib->GetID());

    // copy # of chunks being drawn (parameter buffer) to instance count (DIB)
    data->dibCull->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
    glBindVertexArray(data->vaoCull);
    constexpr GLint offset = offsetof(DrawArraysIndirectCommand, instanceCount);
    glCopyNamedBufferSubData(data->drawCountGPU->GetID(), data->dibCull->GetID(), 0, offset, sizeof(GLuint));
    glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, (void*)0, 1, 0);
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

  uint64_t ChunkRenderer::AllocChunk(std::span<int> vertices, std::span<uint32_t> indices, const AABB& aabb)
  {
    uint64_t vertexBufferHandle{};
    uint64_t indexBufferHandle{};

    // free oldest allocations until there is enough space to allocate this buffer
    //while ((vertexBufferHandle = data->verticesAllocator->Allocate(
    //  vertices.data(),
    //  vertices.size() * sizeof(GLint),
    //  aabb)) == 0)
    //{
    //  data->verticesAllocator->FreeOldest();
    //}

    //while ((indexBufferHandle = data->indicesAllocator->Allocate(
    //  indices.data(),
    //  indices.size() * sizeof(uint32_t))) == 0)
    //{
    //  data->indicesAllocator->FreeOldest();
    //}
    vertexBufferHandle = data->verticesAllocator->Allocate(vertices.data(), vertices.size() * sizeof(GLint), aabb);
    indexBufferHandle = data->indicesAllocator->Allocate(indices.data(), indices.size() * sizeof(uint32_t));
    if (!vertexBufferHandle || !indexBufferHandle)
    {
      data->verticesAllocator->Free(vertexBufferHandle);
      data->indicesAllocator->Free(indexBufferHandle);
      return 0;
    }

    uint64_t allocIndexHandle = data->currentAllocHandle++;
    data->allocHandles.emplace(allocIndexHandle, glm::u64vec2(vertexBufferHandle, indexBufferHandle));
    data->dirtyAlloc = true;

    return allocIndexHandle;
  }

  void ChunkRenderer::FreeChunk(uint64_t allocHandle)
  {
    auto it = data->allocHandles.find(allocHandle);
    if (it == data->allocHandles.end())
      return;
    glm::u64vec2 handles = it->second;
    data->verticesAllocator->Free(handles[0]);
    data->indicesAllocator->Free(handles[1]);
    data->allocHandles.erase(it);
    data->dirtyAlloc = true;
  }

  std::vector<glm::uvec2> ChunkRenderer::GetAllocIndices()
  {
    std::vector<glm::uvec2> vec;
    for (const auto& [key, val] : data->allocHandles)
    {
      glm::uvec2 offsets;
      offsets[0] = data->verticesAllocator->GetAllocOffset(val[0]);
      offsets[1] = data->indicesAllocator->GetAllocOffset(val[1]);
      vec.push_back(offsets);
    }
    return vec;
  }
}