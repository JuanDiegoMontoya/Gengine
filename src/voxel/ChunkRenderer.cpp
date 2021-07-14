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

#include <filesystem>

#include <imgui/imgui.h>

namespace Voxels
{
  // call after all chunks are initialized
  ChunkRenderer::ChunkRenderer()
  {
    drawCountGPU = std::make_unique<GFX::StaticBuffer>(nullptr, sizeof(GLint));

    // allocate big buffer
    // TODO: vary the allocation size based on some user setting
    allocator = std::make_unique<GFX::DynamicBuffer<AABB16>>(10'000'000, 2 * sizeof(GLint));

    /* :::::::::::BUFFER FORMAT:::::::::::
                            CHUNK 1                                    CHUNK 2                   NULL                   CHUNK 3
            | cpos, encoded+lighting, encoded+lighting, ... | cpos, encoded+lighting, ... | null (any length) | cpos, encoded+lighting, ... |
    First:   offset(CHUNK 1)=0                               offset(CHUNK 2)                                   offset(CHUNK 3)
    Draw commands will specify where in memory the draw call starts. This will account for variable offsets.

        :::::::::::BUFFER FORMAT:::::::::::*/
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // bind big data buffer (interleaved)
    glBindBuffer(GL_ARRAY_BUFFER, allocator->GetGPUHandle());
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1); // only 1 instance of a chunk should render, so divisor *should* be infinity
    GLuint offset = 0;
    // stride is sizeof(vertex) so baseinstance can be set to cmd.first and work (hopefully)
    glVertexAttribIPointer(2, 3, GL_INT, 2 * sizeof(GLuint), (void*)offset); // chunk position (one per instance)
    offset += sizeof(glm::ivec4); // move forward by TWO vertex sizes (vertex aligned)

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 2 * sizeof(GLuint), (void*)offset); // encoded data
    offset += 1 * sizeof(GLfloat);

    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 2 * sizeof(GLuint), (void*)offset); // lighting

    // setup vertex buffer for cube that will be used for culling
    glGenVertexArrays(1, &vaoCull);
    //vaoCull->Bind();
    //vboCull = std::make_unique<GFX::StaticBuffer>(Vertices::cube, sizeof(Vertices::cube));
    //vboCull->Bind<GFX::Target::VBO>();
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    //glEnableVertexAttribArray(0);
    //vboCull->Unbind<GFX::Target::VBO>();
    //vaoCull->Unbind();

    DrawElementsIndirectCommand cmd;
    cmd.count = 14; // vertices on cube
    cmd.instanceCount = 0; // will be incremented - reset every frame
    cmd.baseVertex = 0;
    cmd.firstIndex = 0;
    cmd.baseInstance = 0;
    dibCull = std::make_unique<GFX::StaticBuffer>(&cmd, sizeof(cmd), GFX::BufferFlag::CLIENT_STORAGE);

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
    //std::span texSpan(texs.data(), texs.size());
    //textures = std::make_unique<GFX::TextureArray>(std::span(texs.data(), texs.size()), glm::ivec2(32));
    blockTextures = GFX::LoadTexture2DArray(texsView);
    //const auto& texturesInfo = blockTextures->CreateInfo();

    blockTexturesView = GFX::TextureView::Create(*blockTextures);

    GFX::SamplerState ss;
    ss.asBitField.magFilter = GFX::Filter::NEAREST;
    ss.asBitField.minFilter = GFX::Filter::LINEAR;
    ss.asBitField.mipmapFilter = GFX::Filter::LINEAR;
    ss.asBitField.anisotropy = GFX::Anisotropy::SAMPLES_16;
    blockTexturesSampler = GFX::TextureSampler::Create(ss);

    blueNoiseTexture = GFX::LoadTexture2D("BlueNoise/64_64/LDR_LLL1_0.png", GFX::Format::R8G8B8A8_UNORM);
    blueNoiseView = GFX::TextureView::Create(*blueNoiseTexture);
    blueNoiseSampler = GFX::TextureSampler::Create(GFX::SamplerState{});
    //blueNoise64 = std::make_unique<GFX::Texture2D>("BlueNoise/64_64/LDR_LLL1_0.png");
    //blueNoise64 = std::make_unique<Texture2D>("BlueNoise/256_256/LDR_LLL1_0.png");

  }

  ChunkRenderer::~ChunkRenderer()
  {
  }

  void ChunkRenderer::GenerateDrawCommandsGPU()
  {
    GFX::DebugMarker marker("Frustum cull chunks");
    //PERF_BENCHMARK_START;
#ifdef TRACY_ENABLE
    TracyGpuZone("Gen draw commands norm");
#endif

    //Camera* cam = Camera::ActiveCamera;
    auto sdr = GFX::ShaderManager::Get()->GetShader("compact_batch");
    sdr->Bind();

    // set uniforms for chunk rendering
    //sdr->setVec3("u_viewpos", cam->GetPos());
    //Frustum fr = *cam->GetFrustum();
    sdr->SetVec3("u_viewpos", CameraSystem::GetPos());
    Frustum fr = *CameraSystem::GetFrustum();
    for (int i = 0; i < 5; i++) // ignore near plane
    {
      std::string uname = "u_viewfrustum.data_[" + std::to_string(i) + "][0]";
      sdr->Set1FloatArray(hashed_string(uname.c_str()), std::span<float, 4>(fr.GetData()[i]));
    }
    sdr->SetFloat("u_cullMinDist", settings.normalMin);
    sdr->SetFloat("u_cullMaxDist", settings.normalMax);
    sdr->SetUInt("u_reservedVertices", 2);
    sdr->SetUInt("u_vertexSize", sizeof(GLuint) * 2);

    GLint zero = 0;
    drawCountGPU->SubData(&zero, sizeof(GLint));

    const auto& allocs = allocator->GetAllocs();

    // only re-construct if allocator has been modified
    if (dirtyAlloc)
    {
      allocBuffer = std::make_unique<GFX::StaticBuffer>(allocs.data(), allocator->AllocSize() * allocs.size());
      dib = std::make_unique<GFX::StaticBuffer>(nullptr, allocator->ActiveAllocs() * sizeof(DrawArraysIndirectCommand));
      dirtyAlloc = false;
    }

    allocBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
    dib->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
    drawCountGPU->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(2);

    {
      int numBlocks = (allocs.size() + groupSize - 1) / groupSize;
      glDispatchCompute(numBlocks, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // make SSBO writes visible to subsequent execution
    }

    activeAllocs = allocator->ActiveAllocs();

    //PERF_BENCHMARK_END;
  }

  void ChunkRenderer::RenderNorm()
  {
#ifdef TRACY_ENABLE
    TracyGpuZone("Render chunks normal");
#endif
    //if (renderCount == 0)
    //  return;

    glBindVertexArray(vao);
    dib->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
    drawCountGPU->Bind<GFX::Target::PARAMETER_BUFFER>();
    glMultiDrawArraysIndirectCount(GL_TRIANGLES, (void*)0, (GLintptr)0, allocator->ActiveAllocs(), 0);
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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // don't forget to reset original culling face

    // render blocks in each active chunk
    auto currShader = GFX::ShaderManager::Get()->GetShader("chunk_optimized");
    currShader->Bind();

    //Camera* cam = Camera::ActiveCamera;
    //float angle = glm::max(glm::dot(-glm::normalize(NuRenderer::activeSun_->GetDir()), glm::vec3(0, 1, 0)), 0.f);
    static float angle = 2.0f;
    ImGui::SliderFloat("Sunlight strength", &angle, 0.f, 5.f);
    currShader->SetFloat("sunAngle", angle);

    // undo gamma correction for sky color
    static const glm::vec3 skyColor(
      glm::pow(.529f, 2.2f),
      glm::pow(.808f, 2.2f),
      glm::pow(.922f, 2.2f));
    //currShader->setVec3("viewPos", cam->GetPos());
    currShader->SetVec3("viewPos", CameraSystem::GetPos());
    currShader->SetFloat("fogStart", 400.0f);
    currShader->SetFloat("fogEnd", 2000.0f);
    currShader->SetVec3("fogColor", skyColor);
    //currShader->setMat4("u_viewProj", cam->GetProj() * cam->GetView());
    currShader->SetMat4("u_viewProj", CameraSystem::GetProj() * CameraSystem::GetView());

    //textures->Bind(0);
    //currShader->SetInt("textures", 0);
    //static bool filter = true;
    //static float anisotropic = 1.0f;
    //ImGui::Checkbox("Filter textures", &filter);
    //ImGui::SliderFloat("Anisotrpic", &anisotropic, 1.0f, 16.0f);
    //glTextureParameteri(textures->ID(), GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
    //glTextureParameteri(textures->ID(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTextureParameterf(textures->ID(), GL_TEXTURE_MAX_ANISOTROPY, anisotropic);


    RenderVisible();
    GenerateDIB();
    RenderOcclusion();
    //RenderRest();
  }

  void ChunkRenderer::RenderVisible()
  {
    GFX::DebugMarker marker("Draw visible chunks");
    if (!dib)
      return;

#ifdef TRACY_ENABLE
    TracyGpuZone("Normal Render");
#endif

    glBindVertexArray(vao);
    dib->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
    drawCountGPU->Bind<GFX::Target::PARAMETER_BUFFER>();
    blockTexturesView->Bind(0, *blockTexturesSampler);
    blueNoiseView->Bind(1, *blueNoiseSampler);
    glMultiDrawArraysIndirectCount(GL_TRIANGLES, (void*)0, (GLintptr)0, activeAllocs, 0);
    blueNoiseView->Unbind(1);
    blockTexturesView->Unbind(0);
  }

  void ChunkRenderer::GenerateDIB()
  {
    if (settings.freezeCulling)
      return;

    GenerateDrawCommandsGPU();
  }

  void ChunkRenderer::RenderOcclusion()
  {
    GFX::DebugMarker marker("Draw occlusion volumes");
    if (settings.freezeCulling)
      return;

#ifdef TRACY_ENABLE
    TracyGpuZone("Occlusion Render");
#endif

    if (settings.debug_drawOcclusionCulling == false)
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
    sr->SetBool("u_debugDraw", settings.debug_drawOcclusionCulling);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, allocator->GetGPUHandle());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, allocator->GetGPUHandle());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dib->GetID());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dib->GetID());

    // copy # of chunks being drawn (parameter buffer) to instance count (DIB)
    dibCull->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
    glBindVertexArray(vaoCull);
    constexpr GLint offset = offsetof(DrawArraysIndirectCommand, instanceCount);
    glCopyNamedBufferSubData(drawCountGPU->GetID(), dibCull->GetID(), 0, offset, sizeof(GLuint));
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

  void ChunkRenderer::Update()
  {
    if (stateInfo != allocator->GetStateInfo())
    {
      stateInfo = allocator->GetStateInfo();
      dirtyAlloc = true;
    }
    //if (allocator)
    //  allocator->Update();
  }
}