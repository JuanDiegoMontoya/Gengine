#include <Graphics/GraphicsIncludes.h>
#include <Rendering/ChunkRenderer.h>
#include <Graphics/DynamicBuffer.h>
#include <Chunks/Chunk.h>
#include <Systems/Camera.h>
#include <Rendering/Frustum.h>
#include <execution>
#include <Graphics/shader.h>
#include <Graphics/param_bo.h>
#include <Chunks/ChunkStorage.h>
#include <Graphics/Vertices.h>
#include <memory>
#include <Managers/GraphicsManager.h>


namespace ChunkRenderer
{
  // Variables
  namespace
  {
    std::unique_ptr<VAO> vao;
    std::unique_ptr<DIB> dib;

    std::unique_ptr<Param_BO> drawCountGPU;

    // size of compute block (not voxel) for the compute shader
    const int blockSize = 64; // defined in compact_batch.cs

    // resets each frame BEFORE the culling phase
    //GLuint allocDataBuffer = 0;
    std::unique_ptr<VAO> vaoCull;
    std::unique_ptr<VBO> vboCull; // stores only cube vertices
    std::unique_ptr<DIB> dibCull;
    GLsizei activeAllocs;
    std::pair<uint64_t, GLuint> stateInfo { 0, 0 };
    bool dirtyAlloc = true;
    std::unique_ptr<StaticBuffer> allocBuffer;
  }


  // call after all chunks are initialized
  void InitAllocator()
  {
    drawCountGPU = std::make_unique<Param_BO>();

    // allocate big buffer
    // TODO: vary the allocation size based on some user setting
    allocator = std::make_unique<DynamicBuffer<AABB16>>(100'000'000, 2 * sizeof(GLint));
    
    /* :::::::::::BUFFER FORMAT:::::::::::
                            CHUNK 1                                    CHUNK 2                   NULL                   CHUNK 3
           | cpos, encoded+lighting, encoded+lighting, ... | cpos, encoded+lighting, ... | null (any length) | cpos, encoded+lighting, ... |
    First:   offset(CHUNK 1)=0                               offset(CHUNK 2)                                   offset(CHUNK 3)
    Draw commands will specify where in memory the draw call starts. This will account for variable offsets.

       :::::::::::BUFFER FORMAT:::::::::::*/
    vao = std::make_unique<VAO>();
    vao->Bind();
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
    vao->Unbind();

    // setup vertex buffer for cube that will be used for culling
    vaoCull = std::make_unique<VAO>();
    vaoCull->Bind();
    vboCull = std::make_unique<VBO>(Vertices::cube, sizeof(Vertices::cube));
    vboCull->Bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    vboCull->Unbind();
    vaoCull->Unbind();

    DrawArraysIndirectCommand cmd;
    cmd.count = 36; // vertices on cube
    cmd.instanceCount = 0; // will be incremented - reset every frame
    cmd.first = 0;
    cmd.baseInstance = 0;
    dibCull = std::make_unique<DIB>(&cmd, sizeof(cmd), GL_STATIC_COPY);

    dib = std::make_unique<DIB>(nullptr, 0, GL_STATIC_COPY);
  }

  
  void GenerateDrawCommandsGPU()
  {
    //PERF_BENCHMARK_START;
#ifdef TRACY_ENABLE
    TracyGpuZone("Gen draw commands norm");
#endif

    Camera* cam = GetCurrentCamera();
    // make buffer sized as if every allocation was non-null
    auto& sdr = Shader::shaders["compact_batch"];
    sdr->Use();
#if 1
    sdr->setVec3("u_viewpos", cam->GetPos());
    Frustum fr = *cam->GetFrustum();
    for (int i = 0; i < 5; i++) // ignore near plane
    {
      std::string uname = "u_viewfrustum.data_[" + std::to_string(i) + "]";
      sdr->set1FloatArray(entt::hashed_string(uname.c_str()), fr.GetData()[i], 4);
    }
    sdr->setFloat("u_cullMinDist", settings.normalMin);
    sdr->setFloat("u_cullMaxDist", settings.normalMax);
#endif
    sdr->setUInt("u_reservedVertices", 2);
    sdr->setUInt("u_vertexSize", sizeof(GLuint) * 2);

    //drawCounter->Bind(0);
    //drawCounter->Reset();
    drawCountGPU->Reset();

    // copy input data to buffer at binding 0
    //GLuint indata;
    //glGenBuffers(1, &indata);
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, indata);
    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, indata);
    const auto& allocs = allocator->GetAllocs();
    
    //glBufferData(GL_SHADER_STORAGE_BUFFER, allocator->AllocSize() * allocs.size(), allocs.data(), GL_STATIC_COPY);

    // only re-construct if allocator has been modified
    if (dirtyAlloc)
    {
      allocBuffer = std::make_unique<StaticBuffer>(allocs.data(), allocator->AllocSize() * allocs.size());
      dib = std::make_unique<DIB>(
        nullptr,
        allocator->ActiveAllocs() * sizeof(DrawArraysIndirectCommand),
        GL_STATIC_COPY);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, allocBuffer->GetID());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, allocBuffer->GetID());

    // make DIB output SSBO (binding 1) for the shader
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dib->GetID());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dib->GetID());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawCountGPU->GetID());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, drawCountGPU->GetID());
    
    {
      int numBlocks = (allocs.size() + blockSize - 1) / blockSize;
      glDispatchCompute(numBlocks, 1, 1);
      //glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    }
    //renderCount = drawCounter->Get(0); // sync point
    //ASSERT(renderCount <= allocator->ActiveAllocs());
    //glDeleteBuffers(1, &indata);

    drawCountGPU->Unbind();
    activeAllocs = allocator->ActiveAllocs();

    //PERF_BENCHMARK_END;
  }


  void RenderNorm()
  {
#ifdef TRACY_ENABLE
    TracyGpuZone("Render chunks normal");
#endif
    //if (renderCount == 0)
    //  return;

    vao->Bind();
    dib->Bind();
    //glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    //glMultiDrawArraysIndirect(GL_TRIANGLES, (void*)0, renderCount, 0);
    drawCountGPU->Bind();
    glMultiDrawArraysIndirectCount(GL_TRIANGLES, (void*)0, (GLintptr)0, allocator->ActiveAllocs(), 0);
  }


  void DrawBuffers()
  {
    glDisable(GL_DEPTH_TEST);

    auto& sdr = Shader::shaders["buffer_vis"];
    sdr->Use();
    glm::mat4 model(1);
    model = glm::scale(model, { 1, 1, 1 });
    model = glm::translate(model, { -.5, -.90, 0 });
    sdr->setMat4("u_model", model);

    glLineWidth(50);
    //allocator->Draw();
    glLineWidth(2);

    glEnable(GL_DEPTH_TEST);
  }
  
  void Render()
  {
    if (!dib)
      return;

#ifdef TRACY_ENABLE
    TracyGpuZone("Normal Render");
#endif

    vao->Bind();
    dib->Bind();
    //glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    //glMultiDrawArraysIndirect(GL_TRIANGLES, (void*)0, renderCount, 0);
    drawCountGPU->Bind();
    glMultiDrawArraysIndirectCount(GL_TRIANGLES, (void*)0, (GLintptr)0, activeAllocs, 0);
  }


  void GenerateDIB()
  {
    if (settings.freezeCulling)
      return;

    GenerateDrawCommandsGPU();
  }


  void RenderOcclusion()
  {
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

    auto& sr = Shader::shaders["chunk_render_cull"];
    sr->Use();

    Camera* cam = GetCurrentCamera();
    const glm::mat4 viewProj = cam->GetProj() * cam->GetView();
    sr->setMat4("u_viewProj", viewProj);
    sr->setUInt("u_chunk_size", Chunk::CHUNK_SIZE);
    sr->setBool("u_debugDraw", settings.debug_drawOcclusionCulling);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, allocator->GetGPUHandle());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, allocator->GetGPUHandle());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dib->GetID());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dib->GetID());
    
    // copy # of chunks being drawn (parameter buffer) to instance count (DIB)
    dibCull->Bind();
    vaoCull->Bind();
    constexpr GLint offset = offsetof(DrawArraysIndirectCommand, instanceCount);
    glCopyNamedBufferSubData(drawCountGPU->GetID(), dibCull->GetID(), 0, offset, sizeof(GLuint));
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glMultiDrawArraysIndirect(GL_TRIANGLES, (void*)0, 1, 0);

    glEnable(GL_CULL_FACE);
    glDepthMask(true);
    glColorMask(true, true, true, true);
  }

  void Update()
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