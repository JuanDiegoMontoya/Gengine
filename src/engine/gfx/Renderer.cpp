#include "../PCH.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"

#include "ShaderManager.h"
#include "../Camera.h"
#include "Mesh.h"
#include "DebugMarker.h"

#include <engine/ecs/system/ParticleSystem.h>
#include "StaticBuffer.h"

#include <engine/ecs/component/Transform.h>
#include <engine/ecs/component/Rendering.h>
#include <engine/ecs/component/ParticleEmitter.h>
#include "TextureManager.h"
#include "TextureLoader.h"

#include <execution>
#include <iostream>
#include "../CVar.h"
#include "../Console.h"
#include "../Parser.h"

void vsyncCallback([[maybe_unused]] const char* cvar, cvar_float val)
{
  glfwSwapInterval(val != 0); // 0 == no vsync, 1 == vsync
}

void logShaderNames(const char*)
{
  auto shaderNames = GFX::ShaderManager::Get()->GetAllShaderNames();
  for (const auto& name : shaderNames)
  {
    Console::Get()->Log("%s\n", name.c_str());
  }
}

void recompileShader(const char* arg)
{
  CmdParser parser(arg);

  CmdAtom atom = parser.NextAtom();
  std::string* str = std::get_if<std::string>(&atom);
  if (!str)
  {
    Console::Get()->Log("Usage: recompile <string>");
    return;
  }

  auto id = hashed_string(str->c_str());
  if (!GFX::ShaderManager::Get()->GetShader(id))
  {
    Console::Get()->Log("No shader found with name %s\n", str->c_str());
    return;
  }

  if (!GFX::ShaderManager::Get()->RecompileShader(id))
  {
    Console::Get()->Log("Failed to recompile shader %s\n", str->c_str());
    return;
  }
}

AutoCVar<cvar_float> vsync("r.vsync", "- Whether vertical sync is enabled", 0, 0, 1, CVarFlag::NONE, vsyncCallback);

static void GLAPIENTRY
GLerrorCB(GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  [[maybe_unused]] GLsizei length,
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
  GFX::DebugMarker marker("Draw scene objects");
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

  for (int i = 0; auto & [view, sampler] : material->second.viewSamplers)
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

void Renderer::RenderParticleEmitter(const Component::ParticleEmitter& emitter, [[maybe_unused]] const Component::Transform& model)
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

  glCreateTextures(GL_TEXTURE_2D, 1, &ldrColor);
  glTextureStorage2D(ldrColor, 1, GL_RGBA16, fboWidth, fboHeight);
  glCreateFramebuffers(1, &ldrFbo);
  glNamedFramebufferTexture(ldrFbo, GL_COLOR_ATTACHMENT0, ldrColor, 0);
  if (GLenum status = glCheckNamedFramebufferStatus(ldrFbo, GL_FRAMEBUFFER); status != GL_FRAMEBUFFER_COMPLETE)
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

  Console::Get()->RegisterCommand("showShaders", "- Lists all shader names", logShaderNames);
  Console::Get()->RegisterCommand("recompile", "- Recompiles a named shader", recompileShader);

  GFX::TextureManager::Get()->AddTexture("blueNoiseRGB",
    *GFX::LoadTexture2D("BlueNoise/64_64/LDR_RGB1_0.png", GFX::Format::R8G8B8A8_UNORM));
  blueNoiseView = GFX::TextureView::Create(*GFX::TextureManager::Get()->GetTexture("blueNoiseRGB"), "BlueNoiseRGBView");
  GFX::SamplerState samplerState{};
  samplerState.asBitField.addressModeU = GFX::AddressMode::REPEAT;
  samplerState.asBitField.addressModeV = GFX::AddressMode::REPEAT;
  blueNoiseSampler = GFX::TextureSampler::Create(samplerState, "BlueNoiseRGBSampler");
}

void Renderer::CompileShaders()
{
  GFX::ShaderManager::Get()->AddShader("batched",
    {
      { "TexturedMeshBatched.vs.glsl", GFX::ShaderType::VERTEX },
      { "TexturedMesh.fs.glsl", GFX::ShaderType::FRAGMENT }
    });

  GFX::ShaderManager::Get()->AddShader("chunk_optimized",
    {
      { "chunk_optimized.vs.glsl", GFX::ShaderType::VERTEX },
      { "chunk_optimized.fs.glsl", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("compact_batch",
    { { "compact_batch.cs.glsl", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("update_particle_emitter",
    { { "update_particle_emitter.cs.glsl", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("update_particle",
    { { "update_particle.cs.glsl", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("textured_array",
    {
      { "textured_array.vs.glsl", GFX::ShaderType::VERTEX },
      { "textured_array.fs.glsl", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("buffer_vis",
    {
      { "buffer_vis.fs.glsl", GFX::ShaderType::FRAGMENT },
      { "buffer_vis.vs.glsl", GFX::ShaderType::VERTEX }
    });
  GFX::ShaderManager::Get()->AddShader("chunk_render_cull",
    {
      { "chunk_render_cull.vs.glsl", GFX::ShaderType::VERTEX },
      { "chunk_render_cull.fs.glsl", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("particle",
    {
      { "particle.vs.glsl", GFX::ShaderType::VERTEX },
      { "particle.fs.glsl", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("skybox",
    {
      { "skybox.vs.glsl", GFX::ShaderType::VERTEX },
      { "skybox.fs.glsl", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("tonemap",
    {
      { "fullscreen_tri.vs.glsl", GFX::ShaderType::VERTEX },
      { "tonemap.fs.glsl", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("calc_exposure",
    { { "calc_exposure.cs.glsl", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("linearize_log_lum_tex",
    { { "linearize_tex.cs.glsl", GFX::ShaderType::COMPUTE } });
  GFX::ShaderManager::Get()->AddShader("generate_histogram",
    { { "generate_histogram.cs.glsl", GFX::ShaderType::COMPUTE } });

  GFX::ShaderManager::Get()->AddShader("sun",
    {
      { "flat_sun.vs.glsl", GFX::ShaderType::VERTEX },
      { "flat_sun.fs.glsl", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("axis",
    {
      { "axis.vs.glsl", GFX::ShaderType::VERTEX },
      { "axis.fs.glsl", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("flat_color",
    {
      { "flat_color.vs.glsl", GFX::ShaderType::VERTEX },
      { "flat_color.fs.glsl", GFX::ShaderType::FRAGMENT }
    });
  GFX::ShaderManager::Get()->AddShader("fxaa",
    {
      { "fullscreen_tri.vs.glsl", GFX::ShaderType::VERTEX },
      { "fxaa.fs.glsl", GFX::ShaderType::FRAGMENT }
    });
}

void Renderer::DrawAxisIndicator()
{
  GFX::DebugMarker marker("Draw axis indicator");
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
  GFX::DebugMarker marker("Draw skybox");
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
  CameraSystem::ActiveCamera->skyboxTexture->Unbind(0);
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
  ImGui::Checkbox("FXAA", &fxaa.enabled);
  ImGui::Checkbox("Dither", &tonemapDither);
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
  GFX::DebugMarker endframeMarker("Postprocessing");
  glBindFramebuffer(GL_FRAMEBUFFER, ldrFbo);

  {
    GFX::DebugMarker tonemappingMarker("Tone mapping");
    glBindTextureUnit(1, color); // HDR buffer

    const float logLowLum = glm::log(targetLuminance / maxExposure);
    const float logMaxLum = glm::log(targetLuminance / minExposure);
    const int computePixelsX = fboWidth / 4;
    const int computePixelsY = fboHeight / 4;

    {
      GFX::DebugMarker marker("Generate luminance histogram");
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

    {
      GFX::DebugMarker marker("Compute camera exposure");
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

    GFX::DebugMarker marker("Apply tone mapping");
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glViewport(0, 0, fboWidth, fboHeight);
    auto shdr = GFX::ShaderManager::Get()->GetShader("tonemap");
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    shdr->Bind();
    shdr->SetFloat("u_exposureFactor", exposure);
    shdr->SetBool("u_useDithering", tonemapDither);
    shdr->SetBool("u_encodeSRGB", gammaCorrection);
    blueNoiseView->Bind(2, *blueNoiseSampler);
    glBindVertexArray(emptyVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    blueNoiseView->Unbind(2);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  if (fxaa.enabled)
  {
    glBindSampler(0, 0);
    glBindTextureUnit(0, ldrColor);
    GFX::DebugMarker marker("FXAA");
    auto shdr = GFX::ShaderManager::Get()->GetShader("fxaa");
    shdr->Bind();
    shdr->SetVec2("u_invScreenSize", { 1.0f / fboWidth, 1.0f / fboHeight });
    shdr->SetFloat("u_contrastThreshold", fxaa.contrastThreshold);
    shdr->SetFloat("u_relativeThreshold", fxaa.relativeThreshold);
    shdr->SetFloat("u_pixelBlendStrength", fxaa.pixelBlendStrength);
    shdr->SetFloat("u_edgeBlendStrength", fxaa.edgeBlendStrength);
    glViewport(0, 0, windowWidth, windowHeight);
    glBindVertexArray(emptyVao);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
  }
  else
  {
    glBlitNamedFramebuffer(ldrFbo, 0,
      0, 0, fboWidth, fboHeight,
      0, 0, windowWidth, windowHeight,
      GL_COLOR_BUFFER_BIT, GL_LINEAR);
  }
}