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
#include "Fence.h"

#include <execution>
#include <iostream>
#include "../CVar.h"
#include "../Console.h"
#include "../Parser.h"
#include <engine/core/StatMacros.h>

#include <imgui/imgui.h>

#define LOG_PARTICLE_RENDER_TIME 0

DECLARE_FLOAT_STAT(Postprocessing, GPU)
DECLARE_FLOAT_STAT(LuminanceHistogram, GPU)
DECLARE_FLOAT_STAT(CameraExposure, GPU)
DECLARE_FLOAT_STAT(FXAA, GPU)

namespace GFX
{
  void vsyncCallback([[maybe_unused]] const char* cvar, cvar_float val)
  {
    glfwSwapInterval(val != 0); // 0 == no vsync, 1 == vsync
  }

  void setRenderScale([[maybe_unused]] const char* cvar, cvar_float scale)
  {
    Renderer::Get()->SetRenderingScale(static_cast<float>(scale));
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
  AutoCVar<cvar_float> renderScale("r.scale", "- Internal rendering resolution scale", 1.0, 0.01, 4.0, CVarFlag::NONE, setRenderScale);

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

    // ignore insignificant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 0
      )//|| id == 131188 || id == 131186)
      return;

    std::stringstream errStream;
    errStream << "OpenGL Debug message (" << id << "): " << message << '\n';

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             errStream << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   errStream << "Source: Window Manager"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: errStream << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     errStream << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     errStream << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           errStream << "Source: Other"; break;
    }

    errStream << '\n';

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               errStream << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: errStream << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  errStream << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         errStream << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         errStream << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              errStream << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          errStream << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           errStream << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               errStream << "Type: Other"; break;
    }

    errStream << '\n';

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
      errStream << "Severity: high";
      spdlog::critical("{}", errStream.str());
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      errStream << "Severity: medium";
      spdlog::error("{}", errStream.str());
      break;
    case GL_DEBUG_SEVERITY_LOW:
      errStream << "Severity: low";
      spdlog::warn("{}", errStream.str());
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      errStream << "Severity: notification";
      spdlog::debug("{}", errStream.str());
      break;
    }
  }

  //glm::mat4 MVP = CameraSystem::GetViewProj() * modelMatrix;
  void Renderer::BeginBatch(size_t size)
  {
    userCommands.resize(size);
  }

  void Renderer::Submit(const Component::Model& model, const Component::BatchedMesh& mesh, const Component::Material& mat)
  {
    auto index = cmdIndex.fetch_add(1, std::memory_order::memory_order_acq_rel);
    userCommands[index] = BatchDrawCommand{ .mesh = mesh.handle, .material = mat.handle, .modelUniform = model.matrix };
  }

  void Renderer::RenderBatch()
  {
    GFX::DebugMarker marker("Draw batched objects");
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
    ASSERT(MaterialManager::Get()->materials_.contains(mat));
    auto& [id, material] = *MaterialManager::Get()->materials_.find(mat);
    DebugMarker marker(("Batch: " + std::string(material.shaderID)).c_str());

    // generate SSBO w/ uniforms
    StaticBuffer uniformBuffer = GFX::StaticBuffer(uniforms.data(), uniforms.size() * sizeof(UniformData));
    uniformBuffer.Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);

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
    auto shader = GFX::ShaderManager::Get()->GetShader(material.shaderID);
    shader->Bind();

    for (auto& customUniform : material.materialUniforms)
    {
      if (customUniform.Setter)
      {
        customUniform.Setter(customUniform.id, *shader);
      }
    }

    for (int i = 0; auto & [view, sampler] : material.viewSamplers)
    {
      view.Bind(i++, sampler);
    }

    const auto& vp = CameraSystem::GetViewProj();
    shader->SetMat4("u_viewProj", vp);

    glBindVertexArray(batchVAO);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(commands.size()), 0);

    for (int i = 0; auto & [view, sampler] : material.viewSamplers)
    {
      view.Unbind(i++);
    }
  }

  void Renderer::BeginRenderParticleEmitter()
  {
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(emptyVao);
    auto shader = GFX::ShaderManager::Get()->GetShader("particle");
    shader->Bind();
    const auto& v = CameraSystem::GetView();
    shader->SetMat4("u_viewProj", CameraSystem::GetViewProj());
    shader->SetVec3("u_cameraRight", { v[0][0], v[1][0], v[2][0] });
    shader->SetVec3("u_cameraUp", { v[0][1], v[1][1], v[2][1] });
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  }

  void Renderer::RenderParticleEmitter(const Component::ParticleEmitter& emitter, [[maybe_unused]] const Component::Transform& model)
  {
#if LOG_PARTICLE_RENDER_TIME
    GFX::TimerQuery timerQuery;
#endif

    ParticleManager::Get().BindEmitter(emitter.handle);
    glDrawArraysIndirect(GL_TRIANGLE_FAN, 0);

#if LOG_PARTICLE_RENDER_TIME
    printf("Emitter render time: %f ms\n", (double)timerQuery.Elapsed_ns() / 1000000.0);
#endif
  }

  Renderer* Renderer::Get()
  {
    static Renderer renderer{};
    return &renderer;
  }

  GLFWwindow* Renderer::Init()
  {
    window_ = InitWindow();

    // TODO: use dynamically sized buffer
    vertexBuffer = std::make_unique<GFX::DynamicBuffer<>>(100'000'000, sizeof(Vertex));
    indexBuffer = std::make_unique<GFX::DynamicBuffer<>>(100'000'000, sizeof(GLuint));

    InitFramebuffers();
    CompileShaders();
    InitVertexLayouts();

    std::vector<int> zeros(tonemap.NUM_BUCKETS, 0);
    tonemap.exposureBuffer = std::make_unique<GFX::StaticBuffer>(zeros.data(), 2 * sizeof(float));
    tonemap.histogramBuffer = std::make_unique<GFX::StaticBuffer>(zeros.data(), tonemap.NUM_BUCKETS * sizeof(int));

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
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

    Console::Get()->RegisterCommand("showShaders", "- Lists all shader names", logShaderNames);
    Console::Get()->RegisterCommand("recompile", "- Recompiles a named shader", recompileShader);

    GFX::TextureManager::Get()->AddTexture("blueNoiseRGB",
      *GFX::LoadTexture2D("BlueNoise/16_16/LDR_RGB1_0.png", GFX::Format::R8G8B8A8_UNORM));
    tonemap.blueNoiseView = GFX::TextureView::Create(*GFX::TextureManager::Get()->GetTexture("blueNoiseRGB"), "BlueNoiseRGBView");
    GFX::SamplerState samplerState{};
    samplerState.asBitField.addressModeU = GFX::AddressMode::REPEAT;
    samplerState.asBitField.addressModeV = GFX::AddressMode::REPEAT;
    tonemap.blueNoiseSampler.emplace(*GFX::TextureSampler::Create(samplerState, "BlueNoiseRGBSampler"));

    vsync.Set(0);

    return window_;
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
    GFX::ShaderManager::Get()->AddShader("fog",
      {
        { "fullscreen_tri.vs.glsl", GFX::ShaderType::VERTEX },
        { "fog.fs.glsl", GFX::ShaderType::FRAGMENT }
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
    glBlendFunc(GL_ONE, GL_ZERO);
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

  void Renderer::StartFrame()
  {
    GL_ResetState();

    hdrFbo->Bind();
    glClearDepth(0.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Begin("Tonemapping");
    ImGui::Checkbox("FXAA", &fxaa.enabled);
    ImGui::Checkbox("Dither", &tonemap.tonemapDither);
    ImGui::Checkbox("Gamma Correction", &tonemap.gammaCorrection);
    ImGui::SliderFloat("Exposure Factor", &tonemap.exposure, .5f, 2.0f, "%.2f");
    ImGui::SliderFloat("Min Exposure", &tonemap.minExposure, .01f, 30.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Max Exposure", &tonemap.maxExposure, .01f, 30.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Target Luminance", &tonemap.targetLuminance, .1f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Adjustment Speed", &tonemap.adjustmentSpeed, .1f, 10.0f, "%.2f");

    ImGui::Separator();
    ImGui::Text("Fog");
    ImGui::SliderFloat("u_a", &fog.u_a, 0, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("u_b", &fog.u_b, 1.0f, 30000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("u_heightOffset", &fog.u_heightOffset, -100.0f, 0);
    ImGui::SliderFloat("u_fog2Density", &fog.u_fog2Density, 0, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("u_beer", &fog.u_beer, 0, 5.0f);
    ImGui::SliderFloat("u_powder", &fog.u_powder, 0, 5.0f);
    ImGui::End();
  }

  void Renderer::EndFrame(float dt)
  {
    GFX::DebugMarker endframeMarker("Postprocessing");
    MEASURE_GPU_TIMER_STAT(Postprocessing);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // fog
    {
      GFX::DebugMarker fogMarker("Fog");

      fog.framebuffer->Bind();

      Shader shader = *ShaderManager::Get()->GetShader("fog");
      shader.Bind();
      shader.SetMat4("u_invViewProj", glm::inverse(CameraSystem::GetViewProj()));
      shader.SetIVec2("u_viewportSize", { GetRenderWidth(), GetRenderHeight() });
      shader.SetFloat("u_a", fog.u_a);
      shader.SetFloat("u_b", fog.u_b);
      shader.SetFloat("u_heightOffset", fog.u_heightOffset);
      shader.SetFloat("u_fog2Density", fog.u_fog2Density);
      shader.SetVec3("u_envColor", fog.albedo);
      shader.SetFloat("u_beer", fog.u_beer);
      shader.SetFloat("u_powder", fog.u_powder);

      //glBindTextureUnit(0, hdrColorTex);
      //glBindTextureUnit(1, hdrDepthTex);
      hdrColorTexView->Bind(0, *defaultSampler);
      hdrDepthTexView->Bind(1, *defaultSampler);
      glBindVertexArray(emptyVao);
      glDepthMask(GL_FALSE);
      glDisable(GL_CULL_FACE);
      glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    ldrFbo->Bind();

    {
      GFX::DebugMarker tonemappingMarker("Tone mapping");

      const float logLowLum = glm::log(tonemap.targetLuminance / tonemap.maxExposure);
      const float logMaxLum = glm::log(tonemap.targetLuminance / tonemap.minExposure);
      const int computePixelsX = GetRenderWidth() / 4;
      const int computePixelsY = GetRenderHeight() / 4;

      {
        //GFX::TimerQuery timerQuery;
        GFX::DebugMarker marker("Generate luminance histogram");
        MEASURE_GPU_TIMER_STAT(LuminanceHistogram);
        auto hshdr = GFX::ShaderManager::Get()->GetShader("generate_histogram");
        hshdr->Bind();
        hshdr->SetInt("u_hdrBuffer", 1);
        hshdr->SetFloat("u_logLowLum", logLowLum);
        hshdr->SetFloat("u_logMaxLum", logMaxLum);
        const int X_SIZE = 16;
        const int Y_SIZE = 8;
        int xgroups = (computePixelsX + X_SIZE - 1) / X_SIZE;
        int ygroups = (computePixelsY + Y_SIZE - 1) / Y_SIZE;
        tonemap.histogramBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
        //glBindTextureUnit(1, fog.tex);
        fog.texView->Bind(1, *defaultSampler);
        glDispatchCompute(xgroups, ygroups, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        //printf("Histogram time: %f ms\n", (double)timerQuery.Elapsed_ns() / 1000000.0);
      }

      //float expo{};
      //glGetNamedBufferSubData(exposureBuffer->GetID(), 0, sizeof(float), &expo);
      //printf("Exposure: %f\n", expo);

      {
        GFX::DebugMarker marker("Compute camera exposure");
        MEASURE_GPU_TIMER_STAT(CameraExposure);
        //glGenerateTextureMipmap(color);
        tonemap.exposureBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
        tonemap.histogramBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
        //floatBufferOut->Bind<GFX::Target::SSBO>(1);
        auto cshdr = GFX::ShaderManager::Get()->GetShader("calc_exposure");
        cshdr->Bind();
        //cshdr->setFloat("u_targetLuminance", targetLuminance);
        cshdr->SetFloat("u_dt", glm::clamp(dt, 0.001f, 1.0f));
        cshdr->SetFloat("u_adjustmentSpeed", tonemap.adjustmentSpeed);
        cshdr->SetFloat("u_logLowLum", logLowLum);
        cshdr->SetFloat("u_logMaxLum", logMaxLum);
        cshdr->SetFloat("u_targetLuminance", tonemap.targetLuminance);
        cshdr->SetInt("u_numPixels", computePixelsX * computePixelsY);
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
      }

      GFX::DebugMarker marker("Apply tone mapping");
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glViewport(0, 0, GetRenderWidth(), GetRenderHeight());
      auto shdr = GFX::ShaderManager::Get()->GetShader("tonemap");
      glDepthMask(GL_FALSE);
      glDisable(GL_CULL_FACE);
      shdr->Bind();
      shdr->SetFloat("u_exposureFactor", tonemap.exposure);
      shdr->SetBool("u_useDithering", tonemap.tonemapDither);
      shdr->SetBool("u_encodeSRGB", tonemap.gammaCorrection);
      fog.texView->Bind(1, *defaultSampler);
      tonemap.blueNoiseView->Bind(2, *tonemap.blueNoiseSampler);
      glBindVertexArray(emptyVao);
      //glBindTextureUnit(1, fog.tex); // rebind because AMD drivers sus
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glDepthMask(GL_TRUE);
      glEnable(GL_CULL_FACE);
      tonemap.blueNoiseView->Unbind(2);
      fog.texView->Unbind(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);

    if (fxaa.enabled)
    {
      GFX::DebugMarker marker("FXAA");
      MEASURE_GPU_TIMER_STAT(FXAA);

      ldrColorTexView->Bind(0, *defaultSampler);
      auto shdr = GFX::ShaderManager::Get()->GetShader("fxaa");
      shdr->Bind();
      shdr->SetVec2("u_invScreenSize", { 1.0f / GetRenderWidth(), 1.0f / GetRenderHeight() });
      shdr->SetFloat("u_contrastThreshold", fxaa.contrastThreshold);
      shdr->SetFloat("u_relativeThreshold", fxaa.relativeThreshold);
      shdr->SetFloat("u_pixelBlendStrength", fxaa.pixelBlendStrength);
      shdr->SetFloat("u_edgeBlendStrength", fxaa.edgeBlendStrength);
      glBindVertexArray(emptyVao);
      glDepthMask(GL_FALSE);
      glDisable(GL_CULL_FACE);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glDepthMask(GL_TRUE);
      glEnable(GL_CULL_FACE);
      glBindSampler(0, 0);
    }
    else
    {
      glBlitNamedFramebuffer(ldrFbo->GetAPIHandle(), 0,
        0, 0, GetRenderWidth(), GetRenderHeight(),
        0, 0, windowWidth, windowHeight,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
  }

  void Renderer::InitFramebuffers()
  {
    //const int levels = glm::floor(glm::log2(glm::max(fboWidth, fboHeight))) + 1;
    TextureCreateInfo hdrColorTexInfo
    {
      .imageType = ImageType::TEX_2D,
      .format = Format::R16G16B16A16_FLOAT,
      .extent = Extent3D{.width = GetRenderWidth(), .height = GetRenderHeight(), .depth = 1 },
      .mipLevels = 1,
      .arrayLayers = 1
    };
    TextureCreateInfo ldrColorTexInfo
    {
      .imageType = ImageType::TEX_2D,
      .format = Format::R8G8B8A8_UNORM,
      //.format = Format::R16G16B16A16_UNORM,
      .extent {.width = GetRenderWidth(), .height = GetRenderHeight(), .depth = 1 },
      .mipLevels = 1,
      .arrayLayers = 1
    };
    TextureCreateInfo depthTexInfo
    {
      .imageType = ImageType::TEX_2D,
      .format = Format::D32_FLOAT,
      .extent = Extent3D{.width = GetRenderWidth(), .height = GetRenderHeight(), .depth = 1 },
      .mipLevels = 1,
      .arrayLayers = 1
    };

    hdrColorTexMemory.emplace(*Texture::Create(hdrColorTexInfo, "HDR Color Texture"));
    hdrDepthTexMemory.emplace(*Texture::Create(depthTexInfo, "HDR Depth Texture"));
    hdrColorTexView.emplace(*TextureView::Create(*hdrColorTexMemory, "HDR Color View"));
    hdrDepthTexView.emplace(*TextureView::Create(*hdrDepthTexMemory, "HDR Depth View"));
    hdrFbo.emplace();
    hdrFbo->SetAttachment(Attachment::COLOR_0, *hdrColorTexView, 0);
    hdrFbo->SetAttachment(Attachment::DEPTH, *hdrDepthTexView, 0);
    ASSERT(hdrFbo->IsValid());

    ldrColorTexMemory.emplace(*Texture::Create(ldrColorTexInfo, "LDR Color Texture"));
    ldrColorTexView.emplace(*TextureView::Create(*ldrColorTexMemory, "LDR Color View"));
    ldrFbo.emplace();
    ldrFbo->SetAttachment(Attachment::COLOR_0, *ldrColorTexView, 0);
    ASSERT(ldrFbo->IsValid());

    fog.texMemory.emplace(*Texture::Create(hdrColorTexInfo, "Fog Color Texture"));
    fog.texView.emplace(*TextureView::Create(*fog.texMemory, "Fog Texture View"));
    fog.framebuffer.emplace();
    fog.framebuffer->SetAttachment(Attachment::COLOR_0, *fog.texView, 0);
    ASSERT(fog.framebuffer->IsValid());

    defaultSampler.emplace(*TextureSampler::Create({}, "Plain Sampler"));

    glViewport(0, 0, GetRenderWidth(), GetRenderHeight());
  }

  void Renderer::InitVertexLayouts()
  {
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

    glVertexArrayVertexBuffer(batchVAO, 0, vertexBuffer->GetID(), 0, sizeof(Vertex));
    glVertexArrayElementBuffer(batchVAO, indexBuffer->GetID());

    glCreateVertexArrays(1, &emptyVao);
  }

  void Renderer::GL_ResetState()
  {
    // texture unit and sampler bindings (first 8, hopefully more than we'll ever need)
    for (int i = 0; i < 7; i++)
    {
      glBindSampler(i, 0);
      glBindTextureUnit(i, 0);
    }

    // triangle winding
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // depth test
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    
    // blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // buffer bindings
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // vertex array binding
    glBindVertexArray(0);

    // shader program binding
    glUseProgram(0);

    // viewport+clipping
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

    // rasterizer
    glLineWidth(1.0f);
    glPointSize(1.0f);
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, GetRenderWidth(), GetRenderHeight());
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // framebuffer
    hdrFbo->Bind();
  }

  GLFWwindow* Renderer::InitWindow()
  {
    if (!glfwInit())
    {
      return nullptr;
    }

    glfwSetErrorCallback([](int, const char* description)
      {
        spdlog::error("GLFW error: {}", description);
      });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    window_ = glfwCreateWindow(videoMode->width, videoMode->height, "Gengine", nullptr, nullptr);

    if (!window_)
    {
      spdlog::critical("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window_);
    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow*, int width, int height)
      {
        Renderer::Get()->SetFramebufferSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
      });

    //if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    if (!gladLoadGL())
    {
      spdlog::critical("Failed to initialize GLAD");
    }

    return window_;
  }

  void Renderer::SetFramebufferSize(uint32_t width, uint32_t height)
  {
    windowWidth = glm::max(width, 1u);
    windowHeight = glm::max(height, 1u);
    InitFramebuffers();
  }

  void Renderer::SetRenderingScale(float scale)
  {
    renderScale = scale;
    InitFramebuffers();
  }
}