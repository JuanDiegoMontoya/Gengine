#include "EnginePCH.h"
#include "Renderer.h"

#include <CoreEngine/WindowUtils.h>
#include <CoreEngine/Window.h>
#include <CoreEngine/shader.h>
#include <CoreEngine/Camera.h>
#include <CoreEngine/Texture2D.h>
#include <CoreEngine/Mesh.h>
#include <CoreEngine/TextureCube.h>

#include <execution>

static void GLAPIENTRY
GLerrorCB(GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  [[maybe_unused]] const void* userParam)
{
  //return; // UNCOMMENT WHEN DEBUGGING GRAPHICS

  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204
    )//|| id == 131188 || id == 131186)
    return;

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " << message << std::endl;

  switch (source)
  {
  case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window Manager"; break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
  case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
  case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
  } std::cout << std::endl;

  switch (type)
  {
  case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
  case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
  case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
  case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
  case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
  case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
  case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
  } std::cout << std::endl;

  switch (severity)
  {
  case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
  case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
  case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
  case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
  } std::cout << std::endl;
  std::cout << std::endl;
}

//glm::mat4 MVP = CameraSystem::GetViewProj() * modelMatrix;
void Renderer::BeginBatch(size_t size)
{
  userCommands.resize(size);
}

void Renderer::Submit(const Components::Transform& model, const Components::BatchedMesh& mesh, const Components::Material& mat)
{
  auto index = cmdIndex.fetch_add(1, std::memory_order::memory_order_acq_rel);
  userCommands[index] = BatchDrawCommand{ .mesh = mesh.handle->handle, .material = mat.handle->handle, .modelUniform = model.GetModel() };
}

void Renderer::RenderBatch()
{
  userCommands.resize(cmdIndex);
  //cmdIndex.store(0, std::memory_order_release);
  cmdIndex = 0;
  if (userCommands.empty())
  {
    return;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_TRUE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // NOTE: mesh order MUST match meshBufferInfo's
  std::sort(std::execution::par, userCommands.begin(), userCommands.end(),
    [](const auto& lhs, const auto& rhs)
    {
      if (lhs.material != rhs.material)
        return lhs.material < rhs.material;
      else
        return lhs.mesh < rhs.mesh;
    });

  // accumulate per-material draws and uniforms
  std::vector<UniformData> uniforms;
  uniforms.reserve(userCommands.size());
  MaterialID curMat = userCommands[0].material;
  for (size_t i = 0; i < userCommands.size(); i++)
  {
    const auto& draw = userCommands[i];
    if (draw.material != curMat)
    {
      RenderBatchHelper(curMat, uniforms); // submit draw when material is done
      curMat = draw.material;
      uniforms.clear();
    }

    meshBufferInfo[draw.mesh].instanceCount++;
    uniforms.push_back(UniformData{ .model = draw.modelUniform });
  }
  if (uniforms.size() > 0)
  {
    RenderBatchHelper(curMat, uniforms);
  }

  userCommands.clear();
}


void Renderer::RenderBatchHelper(MaterialID mat, const std::vector<UniformData>& uniforms)
{
  // generate SSBO w/ uniforms
  auto uniformBuffer = std::make_unique<GFX::StaticBuffer>(uniforms.data(), uniforms.size() * sizeof(UniformData));
  uniformBuffer->Bind<GFX::Target::SSBO>(0);

  // generate DIB (one indirect command per mesh)
  std::vector<DrawElementsIndirectCommand> commands;
  GLuint baseInstance = 0;
  std::for_each(meshBufferInfo.begin(), meshBufferInfo.end(),
    [&commands, &baseInstance](auto& cmd)
    {
      if (cmd.second.instanceCount != 0)
      {
        cmd.second.baseInstance = baseInstance;
        //cmd.second.instanceCount += baseInstance;
        commands.push_back(cmd.second);
        baseInstance += cmd.second.instanceCount;
      }
    });
  GFX::StaticBuffer dib(commands.data(), commands.size() * sizeof(DrawElementsIndirectCommand));
  dib.Bind<GFX::Target::DIB>();

  // clear instance count for next GL draw command
  for (auto& info : meshBufferInfo)
  {
    info.second.instanceCount = 0;
  }

  // do the actual draw
  auto& material = MaterialManager::materials_[mat];
  auto& shader = Shader::shaders[material.shaderID];
  shader->Use();

  const auto& vp = CameraSystem::GetViewProj();
  shader->setMat4("u_viewProj", vp);

  batchVAO->Bind();
  glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(commands.size()), 0);
  batchVAO->Unbind();
}

void Renderer::BeginRenderParticleEmitter()
{
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);
  auto& shader = Shader::shaders["particle"];
  shader->Use();
  emptyVao->Bind();
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void Renderer::RenderParticleEmitter(const Components::ParticleEmitter& emitter, const Components::Transform& model)
{
  auto& shader = Shader::shaders["particle"];

  const auto& v = CameraSystem::GetView();
  shader->setMat4("u_viewProj", CameraSystem::GetViewProj());
  shader->setInt("u_sprite", 0);
  shader->setVec3("u_cameraRight", v[0][0], v[1][0], v[2][0]);
  shader->setVec3("u_cameraUp", v[0][1], v[1][1], v[2][1]);
  emitter.texture->Bind(0);
  emitter.particleBuffer->Bind<GFX::Target::SSBO>(0);

  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, emitter.maxParticles);
}

void Renderer::Init()
{
  glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

  //const int levels = glm::floor(glm::log2(glm::max(fboWidth, fboHeight))) + 1;
  glCreateTextures(GL_TEXTURE_2D, 1, &color);
  glTextureStorage2D(color, 1, GL_RGBA16F, fboWidth, fboHeight);

  glCreateTextures(GL_TEXTURE_2D, 1, &depth);
  glTextureStorage2D(depth, 1, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight);

  glCreateFramebuffers(1, &fbo);
  glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, color, 0);
  glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, depth, 0);
  if (GLenum status = glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER); status != GL_FRAMEBUFFER_COMPLETE)
  {
    fprintf(stderr, "glCheckNamedFramebufferStatus: %x\n", status);
  }
  //size_t pow2Size = glm::exp2(glm::ceil(glm::log2(double(fboWidth * fboHeight)))); // next power of 2
  std::vector<int> zeros(NUM_BUCKETS, 0);
  //floatBufferIn = std::make_unique<GFX::StaticBuffer>(zeros.data(), pow2Size * sizeof(float), GFX::BufferFlag::CLIENT_STORAGE | GFX::BufferFlag::DYNAMIC_STORAGE);
  //floatBufferOut = std::make_unique<GFX::StaticBuffer>(zeros.data(), pow2Size * sizeof(float), GFX::BufferFlag::CLIENT_STORAGE);
  exposureBuffer = std::make_unique<GFX::StaticBuffer>(&exposure, 2 * sizeof(float));
  histogramBuffer = std::make_unique<GFX::StaticBuffer>(zeros.data(), NUM_BUCKETS * sizeof(int));

  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEPTH_TEST);

  // enable debugging stuff
  glDebugMessageCallback(GLerrorCB, NULL);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glfwSwapInterval(0); // 0 == no vsync, 1 == vsync

  CompileShaders();

  // TODO: use dynamically sized buffer
  vertexBuffer = std::make_unique<GFX::DynamicBuffer<>>(100'000'000, sizeof(Vertex));
  indexBuffer = std::make_unique<GFX::DynamicBuffer<>>(100'000'000, sizeof(GLuint));

  // setup VAO for batched drawing (ONE VERTEX LAYOUT ATM)
  batchVAO = std::make_unique<GFX::VAO>();
  glEnableVertexArrayAttrib(batchVAO->GetID(), 0); // pos
  glEnableVertexArrayAttrib(batchVAO->GetID(), 1); // normal
  glEnableVertexArrayAttrib(batchVAO->GetID(), 2); // uv

  glVertexArrayAttribFormat(batchVAO->GetID(), 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
  glVertexArrayAttribFormat(batchVAO->GetID(), 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  glVertexArrayAttribFormat(batchVAO->GetID(), 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));

  glVertexArrayAttribBinding(batchVAO->GetID(), 0, 0);
  glVertexArrayAttribBinding(batchVAO->GetID(), 1, 0);
  glVertexArrayAttribBinding(batchVAO->GetID(), 2, 0);

  glVertexArrayVertexBuffer(batchVAO->GetID(), 0, vertexBuffer->GetGPUHandle(), 0, sizeof(Vertex));
  glVertexArrayElementBuffer(batchVAO->GetID(), indexBuffer->GetGPUHandle());

  emptyVao = std::make_unique<GFX::VAO>();
  //compute_test();

  /*Layout layout = Window::layout;

  int width = layout.width;
  int height = layout.height;

  // Setup frameBuffer
  glGenFramebuffers(1, &RenderBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, RenderBuffer);

  // Setup render texture
  glGenTextures(1, &RenderTexture);
  glBindTexture(GL_TEXTURE_2D, RenderTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTexture, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer in Raytrace Renderer is not complete!" << std::endl;
  }*/
}

void Renderer::CompileShaders()
{
  Shader::shaders["ShaderMcShaderFace"].emplace(Shader(
    {
      { "TexturedMesh.vs", GL_VERTEX_SHADER },
      { "TexturedMesh.fs", GL_FRAGMENT_SHADER }
    }));
  Shader::shaders["batched"].emplace(Shader(
    {
      { "TexturedMeshBatched.vs", GL_VERTEX_SHADER },
      { "TexturedMesh.fs", GL_FRAGMENT_SHADER }
    }));

  Shader::shaders["chunk_optimized"].emplace(Shader(
    {
      { "chunk_optimized.vs", GL_VERTEX_SHADER },
      { "chunk_optimized.fs", GL_FRAGMENT_SHADER }
    }));
  //Shader::shaders["chunk_splat"] = new Shader("chunk_splat.vs", "chunk_splat.fs");
  Shader::shaders["compact_batch"].emplace(Shader(
    { { "compact_batch.cs", GL_COMPUTE_SHADER } }));
  Shader::shaders["update_particle_emitter"].emplace(Shader(
    { { "update_particle_emitter.cs", GL_COMPUTE_SHADER } }));
  Shader::shaders["update_particle"].emplace(Shader(
    { { "update_particle.cs", GL_COMPUTE_SHADER } }));
  //Shader::shaders["compact_batch"] = new Shader(0, "compact_batch.cs");
  Shader::shaders["textured_array"].emplace(Shader(
    {
      { "textured_array.vs", GL_VERTEX_SHADER },
      { "textured_array.fs", GL_FRAGMENT_SHADER }
    }));
  Shader::shaders["buffer_vis"].emplace(Shader(
    {
      { "buffer_vis.fs", GL_FRAGMENT_SHADER },
      { "buffer_vis.vs", GL_VERTEX_SHADER }
    }));
  Shader::shaders["chunk_render_cull"].emplace(Shader(
    {
      { "chunk_render_cull.vs", GL_VERTEX_SHADER },
      { "chunk_render_cull.fs", GL_FRAGMENT_SHADER }
    }));
  Shader::shaders["particle"].emplace(Shader(
    {
      { "particle.vs", GL_VERTEX_SHADER },
      { "particle.fs", GL_FRAGMENT_SHADER }
    }));
  Shader::shaders["skybox"].emplace(Shader(
    {
      { "skybox.vs", GL_VERTEX_SHADER },
      { "skybox.fs", GL_FRAGMENT_SHADER }
    }));
  Shader::shaders["tonemap"].emplace(Shader(
    {
      { "fullscreen_tri.vs", GL_VERTEX_SHADER },
      { "tonemap.fs", GL_FRAGMENT_SHADER }
    }));
  Shader::shaders["calc_exposure"].emplace(Shader(
    { { "calc_exposure.cs", GL_COMPUTE_SHADER } }));
  Shader::shaders["linearize_log_lum_tex"].emplace(Shader(
    { { "linearize_tex.cs", GL_COMPUTE_SHADER } }));
  Shader::shaders["generate_histogram"].emplace(Shader(
    { { "generate_histogram.cs", GL_COMPUTE_SHADER } }));
  Shader::shaders["reduce_sum_1024"].emplace(Shader(
    { { "reduce_sum.cs", GL_COMPUTE_SHADER } }));
  Shader::shaders["reduce_sum_512"].emplace(Shader(
    { { "reduce_sum.cs", GL_COMPUTE_SHADER, {{"#define WORKGROUP_SIZE 1024", "#define WORKGROUP_SIZE 512"}} } }));
  Shader::shaders["reduce_sum_256"].emplace(Shader(
    { { "reduce_sum.cs", GL_COMPUTE_SHADER, {{"#define WORKGROUP_SIZE 1024", "#define WORKGROUP_SIZE 256"}} } }));
  Shader::shaders["reduce_sum_128"].emplace(Shader(
    { { "reduce_sum.cs", GL_COMPUTE_SHADER, {{"#define WORKGROUP_SIZE 1024", "#define WORKGROUP_SIZE 128"}} } }));
  Shader::shaders["reduce_sum_64"].emplace(Shader(
    { { "reduce_sum.cs", GL_COMPUTE_SHADER, {{"#define WORKGROUP_SIZE 1024", "#define WORKGROUP_SIZE 64"}} } }));
  Shader::shaders["reduce_sum_32"].emplace(Shader(
    { { "reduce_sum.cs", GL_COMPUTE_SHADER, {{"#define WORKGROUP_SIZE 1024", "#define WORKGROUP_SIZE 32"}} } }));

  Shader::shaders["sun"].emplace(Shader("flat_sun.vs", "flat_sun.fs"));
  Shader::shaders["axis"].emplace(Shader("axis.vs", "axis.fs"));
  Shader::shaders["flat_color"].emplace(Shader("flat_color.vs", "flat_color.fs"));
}

void Renderer::DrawAxisIndicator()
{
  static GFX::VAO* axisVAO;
  static GFX::StaticBuffer* axisVBO;
  if (axisVAO == nullptr)
  {
    float indicatorVertices[] =
    {
      // positions      // colors
      0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // x-axis
      1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // y-axis
      0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // z-axis
      0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

    axisVAO = new GFX::VAO();
    axisVBO = new GFX::StaticBuffer(indicatorVertices, sizeof(indicatorVertices));
    GFX::VBOlayout layout;
    layout.Push<float>(3);
    layout.Push<float>(3);
    axisVAO->AddBuffer(*axisVBO, layout);
  }
  auto& currShader = Shader::shaders["axis"];
  currShader->Use();
  //Camera* cam = Camera::ActiveCamera;
  currShader->setMat4("u_model", glm::translate(glm::mat4(1), CameraSystem::GetPos() + CameraSystem::GetFront() * 10.f)); // add scaling factor (larger # = smaller visual)
  currShader->setMat4("u_view", CameraSystem::GetView());
  currShader->setMat4("u_proj", CameraSystem::GetProj());
  glDepthFunc(GL_ALWAYS); // allows indicator to always be rendered
  axisVAO->Bind();
  glLineWidth(2.f);
  glDrawArrays(GL_LINES, 0, 6);
  glDepthFunc(GL_GEQUAL);
  axisVAO->Unbind();
}

// draws a single quad over the entire viewport
void Renderer::DrawQuad()
{
  static unsigned int quadVAO = 0;
  static unsigned int quadVBO;
  if (quadVAO == 0)
  {
    float quadVertices[] =
    {
      // positions        // texture Coords
      -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
       1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
       1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

void Renderer::DrawCube()
{
  static GFX::VAO* blockHoverVao = nullptr;
  static GFX::StaticBuffer* blockHoverVbo = nullptr;
  if (blockHoverVao == nullptr)
  {
    blockHoverVao = new GFX::VAO();
    blockHoverVbo = new GFX::StaticBuffer(Vertices::cube_norm_tex, sizeof(Vertices::cube_norm_tex));
    GFX::VBOlayout layout;
    layout.Push<float>(3);
    layout.Push<float>(3);
    layout.Push<float>(2);
    blockHoverVao->AddBuffer(*blockHoverVbo, layout);
  }
  blockHoverVao->Bind();
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Renderer::DrawSkybox()
{
  auto& shdr = Shader::shaders["skybox"];
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);
  shdr->Use();
  shdr->setMat4("u_proj", CameraSystem::GetProj());
  shdr->setMat4("u_modview", glm::translate(CameraSystem::GetView(), CameraSystem::GetPos()));
  shdr->setInt("u_skybox", 0);
  CameraSystem::ActiveCamera->skybox->Bind(0);
  emptyVao->Bind();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
  glDepthMask(GL_TRUE);
  glEnable(GL_CULL_FACE);
}

#include <imgui/imgui.h>
void Renderer::StartFrame()
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glClearDepth(0.0f);
  auto cc = glm::vec3(.529f, .808f, .922f);
  glClearColor(cc.r, cc.g, cc.b, 1.f);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GEQUAL);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ImGui::Begin("Tonemapping");
  ImGui::Checkbox("Enabled", &tonemapping);
  ImGui::Checkbox("Gamma Correction", &gammaCorrection);
  ImGui::SliderFloat("Exposure Factor", &exposure, .5f, 2.0f, "%.2f");
  ImGui::SliderFloat("Min Exposure", &minExposure, .01f, 30.0f, "%.2f", 2.f);
  ImGui::SliderFloat("Max Exposure", &maxExposure, .01f, 30.0f, "%.2f", 2.f);
  ImGui::SliderFloat("Target Luminance", &targetLuminance, .1f, 10.0f, "%.2f", 2.f);
  ImGui::SliderFloat("Adjustment Speed", &adjustmentSpeed, .1f, 10.0f, "%.2f");
  if (ImGui::Button("Recompile"))
  {
    Shader::shaders["tonemap"].emplace(Shader(
      {
        { "fullscreen_tri.vs", GL_VERTEX_SHADER },
        { "tonemap.fs", GL_FRAGMENT_SHADER }
      }));
    Shader::shaders["calc_exposure"].emplace(Shader(
      { { "calc_exposure.cs", GL_COMPUTE_SHADER } }));
  }
  ImGui::End();
}

void Renderer::EndFrame(float dt)
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  if (gammaCorrection)
  {
    glEnable(GL_FRAMEBUFFER_SRGB);
  }

  if (tonemapping)
  {
    glBindTextureUnit(1, color); // HDR buffer

    const float logLowLum = glm::log(targetLuminance / maxExposure);
    const float logMaxLum = glm::log(targetLuminance / minExposure);

    {
      auto& hshdr = Shader::shaders["generate_histogram"];
      hshdr->Use();
      hshdr->setInt("u_hdrBuffer", 1);
      hshdr->setFloat("u_logLowLum", logLowLum);
      hshdr->setFloat("u_logMaxLum", logMaxLum);
      const int X_SIZE = 32;
      const int Y_SIZE = 32;
      int xgroups = (fboWidth + X_SIZE - 1) / X_SIZE;
      int ygroups = (fboHeight + Y_SIZE - 1) / Y_SIZE;
      histogramBuffer->Bind<GFX::Target::SSBO>(0);
      glDispatchCompute(xgroups, ygroups, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    //float expo{};
    //glGetNamedBufferSubData(exposureBuffer->GetID(), 0, sizeof(float), &expo);
    //printf("Exposure: %f\n", expo);

    // TODO: do this step on the CPU!
    {
      //glGenerateTextureMipmap(color);
      exposureBuffer->Bind<GFX::Target::SSBO>(0);
      histogramBuffer->Bind<GFX::Target::SSBO>(1);
      //floatBufferOut->Bind<GFX::Target::SSBO>(1);
      auto& cshdr = Shader::shaders["calc_exposure"];
      cshdr->Use();
      //cshdr->setFloat("u_targetLuminance", targetLuminance);
      cshdr->setFloat("u_dt", dt);
      cshdr->setFloat("u_adjustmentSpeed", adjustmentSpeed);
      cshdr->setInt("u_hdrBuffer", 1);
      cshdr->setFloat("u_logLowLum", logLowLum);
      cshdr->setFloat("u_logMaxLum", logMaxLum);
      cshdr->setFloat("u_targetLuminance", targetLuminance);
      glDispatchCompute(1, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glViewport(0, 0, fboWidth, fboHeight);
    auto& shdr = Shader::shaders["tonemap"];
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    shdr->Use();
    shdr->setFloat("u_exposureFactor", exposure);
    shdr->setInt("u_hdrBuffer", 1);
    emptyVao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
  }
  else
  {
    glBlitNamedFramebuffer(fbo, 0,
      0, 0, fboWidth, fboHeight,
      0, 0, windowWidth, windowHeight,
      GL_COLOR_BUFFER_BIT, GL_LINEAR);
  }
  glDisable(GL_FRAMEBUFFER_SRGB);
}

size_t invoke_compute(GFX::StaticBuffer& in, GFX::StaticBuffer& out, size_t size)
{
  size_t blockSize{};
  auto& rshdr = [size, &blockSize]() -> auto&
  {
    if (size > 1024)
    {
      blockSize = 1024;
      return Shader::shaders["reduce_sum_1024"];
    }

    blockSize = size;
    switch (size)
    {
    case 1024: return Shader::shaders["reduce_sum_1024"];
    case 512: return Shader::shaders["reduce_sum_512"];
    case 256: return Shader::shaders["reduce_sum_256"];
    case 128: return Shader::shaders["reduce_sum_128"];
    case 64: return Shader::shaders["reduce_sum_64"];
    default: return Shader::shaders["reduce_sum_32"];
    }
  }();
  rshdr->Use();
  //if (blockSize < 64)
  {
    rshdr->setUInt("u_n", static_cast<unsigned>(size));
  }
  in.Bind<GFX::Target::SSBO>(0);
  out.Bind<GFX::Target::SSBO>(1);
  size_t totalBlocks = (size + blockSize - 1) / blockSize;
  totalBlocks = totalBlocks / 2 + totalBlocks % 2;
  glDispatchCompute(static_cast<GLuint>(totalBlocks), 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  return totalBlocks;
}

void compute_test()
{
  const uint64_t WIDTH = 1 * 1920;
  const uint64_t HEIGHT = 1080;

  size_t pow2Size = size_t(glm::exp2(glm::ceil(glm::log2(double(WIDTH * HEIGHT))))); // next power of 2
  //size_t pow2Size = (WIDTH * HEIGHT) + (WIDTH * HEIGHT) % 1024; // next multiple of 1024
  //size_t pow2Size = 1024*1024;
  std::vector<float> zeros(pow2Size, 0.0f);
  GFX::StaticBuffer inData(zeros.data(), pow2Size * sizeof(float), GFX::BufferFlag::CLIENT_STORAGE | GFX::BufferFlag::DYNAMIC_STORAGE);
  GFX::StaticBuffer outData(zeros.data(), pow2Size * sizeof(float), GFX::BufferFlag::CLIENT_STORAGE | GFX::BufferFlag::DYNAMIC_STORAGE);

  // create a test image with pixels of all the same color
  std::vector<glm::vec4> pixels(WIDTH * HEIGHT, glm::vec4(1.5));
  float sum = 0.0f;
  std::for_each(pixels.begin(), pixels.end(), [&sum](glm::vec4& pix) { pix += glm::sin(sum += .1f); });
  GLuint testTex;
  glCreateTextures(GL_TEXTURE_2D, 1, &testTex);
  glTextureStorage2D(testTex, 1, GL_RGBA16F, WIDTH, HEIGHT);
  glTextureSubImage2D(testTex, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA, GL_FLOAT, pixels.data());
  glBindTextureUnit(1, testTex);

  // check that texture was uploaded correctly
  std::vector<glm::vec4> pixelsOut(WIDTH * HEIGHT, glm::vec4(0));
  glGetTextureSubImage(testTex, 0, 0, 0, 0, WIDTH, HEIGHT, 1, GL_RGBA, GL_FLOAT, static_cast<GLsizei>(pixelsOut.size() * sizeof(glm::vec4)), pixelsOut.data());
  for (int i = 0; i < WIDTH * HEIGHT; i++)
  {
    ASSERT(glm::all(glm::epsilonEqual(pixels[i], pixelsOut[i], glm::vec4(.01f))));
  }

  GFX::Fence fencea;
  fencea.Sync();
  Timer timerLin;
  // take the natural log of each pixel's lumiance, then store it in a flat buffer
  {
    // put the log of each pixel's luminance of the hdr texture into a flat float buffer before reduction
    auto& lshdr = Shader::shaders["linearize_log_lum_tex"];
    lshdr->Use();
    lshdr->setInt("u_hdrBuffer", 1);
    inData.Bind<GFX::Target::SSBO>(0);
    const uint32_t N_L_T = 256; // NUM_LOCAL_THREADS
    const uint32_t numPixels = WIDTH * HEIGHT;
    const uint32_t numGroupsL = (numPixels + N_L_T - 1) / N_L_T;
    glDispatchCompute(numGroupsL, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }
  GFX::Fence fence;
  fence.Sync();
  printf("Linearize time: %fms\n", timerLin.elapsed() * 1000);
  // check previous CS invocation with simple scalar code
  std::vector<float> bufferOut(WIDTH * HEIGHT, 0.0f);
  glGetNamedBufferSubData(inData.GetID(), 0, bufferOut.size() * sizeof(float), bufferOut.data());
  //for (int i = 0; i < WIDTH * HEIGHT; i++)
  //{
  //  float lum = glm::dot(glm::vec3(pixels[i]), glm::vec3(.3f, .59f, .11f));
  //  ASSERT(glm::epsilonEqual(bufferOut[i], glm::log(lum), .001f));
  //}
  GFX::TimerQuery qt;
  constexpr size_t minSize = 32; // GPU will not reduce to fewer than this number of elements
  while (pow2Size > minSize)
  {
    pow2Size = invoke_compute(inData, outData, pow2Size);
    if (pow2Size > minSize) std::swap(inData, outData);
  }
  double timed = qt.Elapsed() / 1000000000.0;
  //float gpuSum{};
  //glGetNamedBufferSubData(outData.GetID(), 0, sizeof(float), &gpuSum);
  std::array<float, minSize> gpuData;
  glGetNamedBufferSubData(outData.GetID(), 0, pow2Size * sizeof(float), gpuData.data());
  float gpuSum = std::reduce(std::execution::par_unseq, gpuData.begin(), gpuData.begin() + pow2Size);
  //double timed = timer.elapsed();
  //timer.reset();
  Timer timer;
  float cpuSum = std::reduce(std::execution::par_unseq, bufferOut.begin(), bufferOut.end());
  double cpuTime = timer.elapsed();
  printf("CPU: %f\nGPU: %f\nCPU time: %.3fms\nGPU time: %.3fms", cpuSum, gpuSum, cpuTime * 1000.0, timed * 1000.0);
}

void histogram_test()
{
  const uint64_t WIDTH = 1 * 1920;
  const uint64_t HEIGHT = 1080;

  size_t size = WIDTH * HEIGHT;
  const size_t NUM_BUCKETS = 128;
  std::vector<int> zeros(size, 0);
  GFX::StaticBuffer histogram(zeros.data(), NUM_BUCKETS * sizeof(int));
  //GFX::StaticBuffer outData(zeros.data(), pow2Size * sizeof(float), GFX::BufferFlag::CLIENT_STORAGE | GFX::BufferFlag::DYNAMIC_STORAGE);

  // create a test image with pixels of all the same color
  std::vector<glm::vec4> pixels(WIDTH * HEIGHT, glm::vec4(1.5));
  float sum = 0.0f;
  std::for_each(pixels.begin(), pixels.end(), [&sum](glm::vec4& pix) { pix += glm::sin(sum += .1f); });
  GLuint testTex;
  glCreateTextures(GL_TEXTURE_2D, 1, &testTex);
  glTextureStorage2D(testTex, 1, GL_RGBA16F, WIDTH, HEIGHT);
  glTextureSubImage2D(testTex, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA, GL_FLOAT, pixels.data());
  glBindTextureUnit(1, testTex);
}