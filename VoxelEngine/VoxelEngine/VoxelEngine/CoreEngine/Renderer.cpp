#include "PCH.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"

#include "ShaderManager.h"
#include "Camera.h"
#include "Texture2D.h"
#include "Mesh.h"
#include "TextureCube.h"
#include "DebugMarker.h"

#include "ParticleSystem.h"
#include "StaticBuffer.h"

#include "Components/Transform.h"
#include "Components/Rendering.h"
#include "Components/ParticleEmitter.h"

#include <execution>
#include <iostream>
#include "CVar.h"

void vsyncCallback(const char* cvar, cvar_float val)
{
  glfwSwapInterval(val != 0); // 0 == no vsync, 1 == vsync
}

AutoCVar<cvar_float> vsync("r.vsync", "- Whether vertical sync is enabled", 0.0, CVarFlag::NONE, vsyncCallback);

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
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 0
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

void Renderer::Submit(const Component::Transform& model, const Component::BatchedMesh& mesh, const Component::Material& mat)
{
  auto index = cmdIndex.fetch_add(1, std::memory_order::memory_order_acq_rel);
  userCommands[index] = BatchDrawCommand{ .mesh = mesh.handle, .material = mat.handle, .modelUniform = model.GetModel() };
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
  uniformBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);

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
  dib.Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();

  // clear instance count for next GL draw command
  for (auto& info : meshBufferInfo)
  {
    info.second.instanceCount = 0;
  }

  // do the actual draw
  auto material = MaterialManager::Get()->materials_.find(mat);
  auto shader = GFX::ShaderManager::Get()->GetShader(material->second.shaderID);
  shader->Bind();

  for (int i = 0; auto& [view, sampler] : material->second.viewSamplers)
  {
    view.Bind(i++, sampler);
  }

  const auto& vp = CameraSystem::GetViewProj();
  shader->SetMat4("u_viewProj", vp);

  glBindVertexArray(batchVAO);
  glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(commands.size()), 0);

  for (int i = 0; auto & [view, sampler] : material->second.viewSamplers)
  {
    view.Unbind(i++);
  }
}

void Renderer::BeginRenderParticleEmitter()
{
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);
  auto shader = GFX::ShaderManager::Get()->GetShader("particle");
  shader->Bind();
  glBindVertexArray(emptyVao);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void Renderer::RenderParticleEmitter(const Component::ParticleEmitter& emitter, const Component::Transform& model)
{
  auto shader = GFX::ShaderManager::Get()->GetShader("particle");

  const auto& v = CameraSystem::GetView();
  shader->SetMat4("u_viewProj", CameraSystem::GetViewProj());
  shader->SetInt("u_sprite", 0);
  shader->SetVec3("u_cameraRight", { v[0][0], v[1][0], v[2][0] });
  shader->SetVec3("u_cameraUp", { v[0][1], v[1][1], v[2][1] });

  ParticleManager::Get().BindEmitter(emitter.handle);
  //glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, emitter.maxParticles);
  glDrawArraysIndirect(GL_TRIANGLE_FAN, 0);
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
  exposureBuffer = std::make_unique<GFX::StaticBuffer>(zeros.data(), 2 * sizeof(float));
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
  glCreateVertexArrays(1, &batchVAO);
  glEnableVertexArrayAttrib(batchVAO, 0); // pos
  glEnableVertexArrayAttrib(batchVAO, 1); // normal
  glEnableVertexArrayAttrib(batchVAO, 2); // uv

  glVertexArrayAttribFormat(batchVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
  glVertexArrayAttribFormat(batchVAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  glVertexArrayAttribFormat(batchVAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));

  glVertexArrayAttribBinding(batchVAO, 0, 0);
  glVertexArrayAttribBinding(batchVAO, 1, 0);
  glVertexArrayAttribBinding(batchVAO, 2, 0);

  glVertexArrayVertexBuffer(batchVAO, 0, vertexBuffer->GetGPUHandle(), 0, sizeof(Vertex));
  glVertexArrayElementBuffer(batchVAO, indexBuffer->GetGPUHandle());

  glCreateVertexArrays(1, &emptyVao);
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
  GFX::ShaderManager::Get()->AddShader("batched",
    {
      { "TexturedMeshBatched.vs", GFX::ShaderType::VERTEX },
      { "TexturedMesh.fs", GFX::ShaderType::FRAGMENT }
    });

  GFX::ShaderManager::Get()->AddShader("chunk_optimized",
    {
      { "chunk_optimized.vs", GFX::ShaderType::VERTEX },
      { "chunk_optimized.fs", GFX::ShaderType::FRAGMENT }
    });
  //GFX::ShaderManager::Get()->AddShader("chunk_splat"] = new Shader("chunk_splat.vs", "chunk_splat.fs");
  GFX::ShaderManager::Get()->AddShader("compact_batch",
    { { "compact_batch.cs", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("update_particle_emitter",
    { { "update_particle_emitter.cs", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("update_particle",
    { { "update_particle.cs", GFX::ShaderType::COMPUTE } });
  //GFX::ShaderManager::Get()->AddShader("compact_batch"] = new Shader(0, "compact_batch.cs");
  GFX::ShaderManager::Get()->AddShader("textured_array",
    {
      { "textured_array.vs", GFX::ShaderType::VERTEX },
      { "textured_array.fs", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("buffer_vis",
    {
      { "buffer_vis.fs", GFX::ShaderType::FRAGMENT },
      { "buffer_vis.vs", GFX::ShaderType::VERTEX }
    });
  GFX::ShaderManager::Get()->AddShader("chunk_render_cull",
    {
      { "chunk_render_cull.vs", GFX::ShaderType::VERTEX },
      { "chunk_render_cull.fs", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("particle",
    {
      { "particle.vs", GFX::ShaderType::VERTEX },
      { "particle.fs", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("skybox",
    {
      { "skybox.vs", GFX::ShaderType::VERTEX },
      { "skybox.fs", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("tonemap",
    {
      { "fullscreen_tri.vs", GFX::ShaderType::VERTEX },
      { "tonemap.fs", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("calc_exposure",
    { { "calc_exposure.cs", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("linearize_log_lum_tex",
    { { "linearize_tex.cs", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("generate_histogram",
    { { "generate_histogram.cs", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("reduce_sum_1024",
    { { "reduce_sum.cs", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("reduce_sum_512",
    { { "reduce_sum.cs", GFX::ShaderType::COMPUTE, {{"#define WORKGROUP_SIZE 1024", "#define WORKGROUP_SIZE 512"}} } });
  GFX::ShaderManager::Get()->AddShader("reduce_sum_256",
    { { "reduce_sum.cs", GFX::ShaderType::COMPUTE, {{"#define WORKGROUP_SIZE 1024", "#define WORKGROUP_SIZE 256"}} } });
  GFX::ShaderManager::Get()->AddShader("reduce_sum_128",
    { { "reduce_sum.cs", GFX::ShaderType::COMPUTE, {{"#define WORKGROUP_SIZE 1024", "#define WORKGROUP_SIZE 128"}} } });
  GFX::ShaderManager::Get()->AddShader("reduce_sum_64",
    { { "reduce_sum.cs", GFX::ShaderType::COMPUTE, {{"#define WORKGROUP_SIZE 1024", "#define WORKGROUP_SIZE 64"}} } });
  GFX::ShaderManager::Get()->AddShader("reduce_sum_32",
    { { "reduce_sum.cs", GFX::ShaderType::COMPUTE, {{"#define WORKGROUP_SIZE 1024", "#define WORKGROUP_SIZE 32"}} } });

  // TODO: convert these to new create method
  GFX::ShaderManager::Get()->AddShader("sun",
    {
      { "flat_sun.vs", GFX::ShaderType::VERTEX },
      { "flat_sun.fs", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("axis",
    {
      { "axis.vs", GFX::ShaderType::VERTEX },
      { "axis.fs", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("flat_color",
    {
      { "flat_color.vs", GFX::ShaderType::VERTEX },
      { "flat_color.fs", GFX::ShaderType::FRAGMENT }
    });
  //GFX::ShaderManager::Get()->AddShader("sun","flat_sun.vs", "flat_sun.fs");
  //GFX::ShaderManager::Get()->AddShader("axis","axis.vs", "axis.fs"));
  //GFX::ShaderManager::Get()->AddShader("flat_color","flat_color.vs", "flat_color.fs");
}

void Renderer::DrawAxisIndicator()
{
  GFX::DebugMarker marker("Axis Indicator");
  static GLuint axisVAO{ 0 };
  static GFX::StaticBuffer* axisVBO{};
  if (axisVAO == 0)
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

    glGenVertexArrays(1, &axisVAO);
    glBindVertexArray(axisVAO);
    axisVBO = new GFX::StaticBuffer(indicatorVertices, sizeof(indicatorVertices));
    axisVBO->Bind<GFX::Target::VERTEX_BUFFER>();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  }
  auto currShader = GFX::ShaderManager::Get()->GetShader("axis");
  currShader->Bind();
  //Camera* cam = Camera::ActiveCamera;
  currShader->SetMat4("u_model", glm::translate(glm::mat4(1), CameraSystem::GetPos() + CameraSystem::GetFront() * 10.f)); // add scaling factor (larger # = smaller visual)
  currShader->SetMat4("u_view", CameraSystem::GetView());
  currShader->SetMat4("u_proj", CameraSystem::GetProj());
  glDepthFunc(GL_ALWAYS); // allows indicator to always be rendered
  glBindVertexArray(axisVAO);
  glLineWidth(2.f);
  glDrawArrays(GL_LINES, 0, 6);
  glDepthFunc(GL_GEQUAL);
  glBindVertexArray(0);
}

void Renderer::DrawSkybox()
{
  auto shdr = GFX::ShaderManager::Get()->GetShader("skybox");
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);
  shdr->Bind();
  shdr->SetMat4("u_proj", CameraSystem::GetProj());
  shdr->SetMat4("u_modview", glm::translate(CameraSystem::GetView(), CameraSystem::GetPos()));
  shdr->SetInt("u_skybox", 0);
  //CameraSystem::ActiveCamera->skybox->Bind(0);
  CameraSystem::ActiveCamera->skyboxTexture->Bind(0, *CameraSystem::ActiveCamera->skyboxSampler);
  glBindVertexArray(emptyVao);
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
  ImGui::SliderFloat("Min Exposure", &minExposure, .01f, 30.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
  ImGui::SliderFloat("Max Exposure", &maxExposure, .01f, 30.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
  ImGui::SliderFloat("Target Luminance", &targetLuminance, .1f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
  ImGui::SliderFloat("Adjustment Speed", &adjustmentSpeed, .1f, 10.0f, "%.2f");
  if (ImGui::Button("Recompile"))
  {
    GFX::ShaderManager::Get()->AddShader("tonemap",
      {
        { "fullscreen_tri.vs", GFX::ShaderType::VERTEX },
        { "tonemap.fs", GFX::ShaderType::FRAGMENT }
      });
  GFX::ShaderManager::Get()->AddShader("calc_exposure",
    { { "calc_exposure.cs", GFX::ShaderType::COMPUTE } });
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
    const int computePixelsX = fboWidth / 4;
    const int computePixelsY = fboHeight / 4;

    {
      auto hshdr = GFX::ShaderManager::Get()->GetShader("generate_histogram");
      hshdr->Bind();
      hshdr->SetInt("u_hdrBuffer", 1);
      hshdr->SetFloat("u_logLowLum", logLowLum);
      hshdr->SetFloat("u_logMaxLum", logMaxLum);
      const int X_SIZE = 8;
      const int Y_SIZE = 8;
      int xgroups = (computePixelsX + X_SIZE - 1) / X_SIZE;
      int ygroups = (computePixelsY + Y_SIZE - 1) / Y_SIZE;
      histogramBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
      glDispatchCompute(xgroups, ygroups, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    //float expo{};
    //glGetNamedBufferSubData(exposureBuffer->GetID(), 0, sizeof(float), &expo);
    //printf("Exposure: %f\n", expo);

    // TODO: do this step on the CPU!
    {
      //glGenerateTextureMipmap(color);
      exposureBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
      histogramBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
      //floatBufferOut->Bind<GFX::Target::SSBO>(1);
      auto cshdr = GFX::ShaderManager::Get()->GetShader("calc_exposure");
      cshdr->Bind();
      //cshdr->setFloat("u_targetLuminance", targetLuminance);
      cshdr->SetFloat("u_dt", glm::clamp(dt, 0.001f, 1.0f));
      cshdr->SetFloat("u_adjustmentSpeed", adjustmentSpeed);
      cshdr->SetFloat("u_logLowLum", logLowLum);
      cshdr->SetFloat("u_logMaxLum", logMaxLum);
      cshdr->SetFloat("u_targetLuminance", targetLuminance);
      cshdr->SetInt("u_numPixels", computePixelsX * computePixelsY);
      glDispatchCompute(1, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glViewport(0, 0, fboWidth, fboHeight);
    auto shdr = GFX::ShaderManager::Get()->GetShader("tonemap");
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    shdr->Bind();
    shdr->SetFloat("u_exposureFactor", exposure);
    shdr->SetInt("u_hdrBuffer", 1);
    glBindVertexArray(emptyVao);
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