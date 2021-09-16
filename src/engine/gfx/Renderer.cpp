#include "../PCH.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/norm.hpp>
#include "Renderer.h"

#include "ShaderManager.h"
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
#include "Camera.h"

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

  // TODO: for proper fullscreen toggling, we need to re-init the other consumers of `window`
  // idea: use callbacks that we call whenever the window is re-initialized
  void fullscreenCallback(const char*, cvar_float val)
  {
    bool fullscreen{ val != 0 };
    if (fullscreen == Renderer::Get()->GetIsFullscreen())
      return;
    Renderer::Get()->CreateWindow(fullscreen);
    Renderer::Get()->InitFramebuffers();
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
      Console::Get()->Log("No shader found with name %s", str->c_str());
      return;
    }

    if (!GFX::ShaderManager::Get()->RecompileShader(id))
    {
      Console::Get()->Log("Failed to recompile shader %s", str->c_str());
      return;
    }
  }

  AutoCVar<cvar_float> vsyncCvar("r.vsync", "- Whether vertical sync is enabled", 0, 0, 1, CVarFlag::NONE, vsyncCallback);
  AutoCVar<cvar_float> renderScaleCvar("r.scale", "- Internal rendering resolution scale", 1.0, 0.1, 2.0, CVarFlag::NONE, setRenderScale);
  //AutoCVar<cvar_float> fullscreenCvar("r.fullscreen", "- Whether the window is fullscreen", 0, 0, 1, CVarFlag::NONE, fullscreenCallback);

  static void GLAPIENTRY GLerrorCB(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    [[maybe_unused]] GLsizei length,
    const GLchar* message,
    [[maybe_unused]] const void* userParam)
  {
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

  void GFX::Renderer::BeginObjects(size_t maxDraws)
  {
    cmdIndex = 0;
    userCommands.resize(maxDraws);
  }

  void Renderer::SubmitObject(const Component::Model& model, const Component::BatchedMesh& mesh, const Component::Material& mat)
  {
    auto index = cmdIndex.fetch_add(1, std::memory_order::memory_order_acq_rel);
    userCommands[index] = BatchDrawCommand{ .mesh = mesh.handle, .material = mat.handle, .modelUniform = model.matrix };
  }

  void Renderer::RenderObjects(std::span<RenderView> renderViews)
  {
    GFX::DebugMarker marker("Draw batched objects");

    userCommands.resize(cmdIndex);
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
        RenderBatchHelper(renderViews, curMat, uniforms); // submit draw when material is done
        curMat = draw.material;
        uniforms.clear();
      }

      meshBufferInfo[draw.mesh].instanceCount++;
      uniforms.push_back(UniformData{ .model = draw.modelUniform });
    }
    if (uniforms.size() > 0)
    {
      RenderBatchHelper(renderViews, curMat, uniforms);
    }

    userCommands.clear();
  }

  void Renderer::RenderBatchHelper(std::span<RenderView> renderViews, MaterialID mat, const std::vector<UniformData>& uniforms)
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

    for (int i = 0; auto& [view, sampler] : material.viewSamplers)
    {
      view.Bind(i++, sampler);
    }

    // TODO: perform GPU culling somewhere around here

    for (auto& renderView : renderViews)
    {
      if (!(renderView.mask & RenderMaskBit::RenderObjects))
        continue;

      renderView.renderTarget->Bind();
      shader->SetMat4("u_viewProj", renderView.camera->GetViewProj());

      glViewport(renderView.offset.x, renderView.offset.y, renderView.size.width, renderView.size.height);
      glBindVertexArray(batchVAO);
      glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(commands.size()), 0);
    }

    for (int i = 0; auto& [view, sampler] : material.viewSamplers)
    {
      view.Unbind(i++);
    }
  }

  void GFX::Renderer::BeginEmitters(size_t maxDraws)
  {
    emitterDrawIndex = 0;
    emitterDrawCommands.resize(maxDraws);
  }

  void GFX::Renderer::SubmitEmitter(const Component::ParticleEmitter& emitter, const Component::Transform& model)
  {
    auto index = emitterDrawIndex.fetch_add(1);
    emitterDrawCommands[index] = { &emitter, model.GetModel() };
  }

  void GFX::Renderer::RenderEmitters(std::span<RenderView> renderViews)
  {
    emitterDrawCommands.resize(emitterDrawIndex);
    if (emitterDrawCommands.empty())
    {
      return;
    }

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBindVertexArray(emptyVao);
    auto shader = GFX::ShaderManager::Get()->GetShader("particle");
    shader->Bind();

    for (auto& renderView : renderViews)
    {
      if (!(renderView.mask & RenderMaskBit::RenderEmitters))
        continue;

      glViewport(renderView.offset.x, renderView.offset.y, renderView.size.width, renderView.size.height);

      auto compare = [&renderView](const EmitterDrawCommand& p1, EmitterDrawCommand& p2)
      {
        auto pos1 = glm::vec3(p1.modelUniform[3]);
        auto pos2 = glm::vec3(p2.modelUniform[3]);
        if (pos1 != pos2)
        {
          auto len = glm::length2(pos1 - renderView.camera->viewInfo.position) -
            glm::length2(pos2 - renderView.camera->viewInfo.position);
          if (glm::abs(len) > 0.001f)
          {
            return len > 0.0f;
          }
        }

        return p1.emitter < p2.emitter;
      };

      std::sort(emitterDrawCommands.begin(), emitterDrawCommands.end(), compare);

      auto v = renderView.camera->viewInfo.GetViewMatrix();
      shader->SetMat4("u_viewProj", renderView.camera->GetViewProj());
      shader->SetVec3("u_cameraRight", { v[0][0], v[1][0], v[2][0] });
      shader->SetVec3("u_cameraUp", { v[0][1], v[1][1], v[2][1] });

      for (auto& [emitter, model] : emitterDrawCommands)
      {
        // TODO: perform culling on the emitter, run the per-particle culling compute shader
        //if () // special path for if particles move in local space rather than world space
        ParticleManager::Get().BindEmitter(emitter->handle);
        glDrawArraysIndirect(GL_TRIANGLE_FAN, 0);
      }
    }
  }

  void GFX::Renderer::DrawFog(std::span<RenderView> renderViews)
  {
    GFX::DebugMarker fogMarker("Fog");

    Shader shader = *ShaderManager::Get()->GetShader("fog");
    shader.Bind();
    shader.SetFloat("u_a", fog.u_a);
    shader.SetFloat("u_b", fog.u_b);
    shader.SetFloat("u_heightOffset", fog.u_heightOffset);
    shader.SetFloat("u_fog2Density", fog.u_fog2Density);
    shader.SetVec3("u_envColor", fog.albedo);
    shader.SetFloat("u_beer", fog.u_beer);
    shader.SetFloat("u_powder", fog.u_powder);
    
    glBindVertexArray(emptyVao);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_ONE, GL_ZERO);

    for (auto& renderView : renderViews)
    {
      if (!(renderView.mask & RenderMaskBit::RenderFog))
        continue;

      glViewport(renderView.offset.x, renderView.offset.y, renderView.size.width, renderView.size.height);
      shader.SetIVec2("u_viewportSize", { renderView.size.width, renderView.size.height });

      // yes, we are reading from the render target while drawing to it
      // this use case is valid under ARB_texture_barrier
      // see here: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_barrier.txt
      renderView.renderTarget->Bind();
      shader.SetMat4("u_invViewProj", glm::inverse(renderView.camera->GetViewProj()));

      GFX::BindTextureViewNative(0,
        renderView.renderTarget->GetAttachmentAPIHandle(Attachment::COLOR_0),
        defaultSampler->GetAPIHandle());
      GFX::BindTextureViewNative(1,
        renderView.renderTarget->GetAttachmentAPIHandle(Attachment::DEPTH),
        defaultSampler->GetAPIHandle());

      glDrawArrays(GL_TRIANGLES, 0, 3);
    }
  }

  Renderer* Renderer::Get()
  {
    static Renderer renderer{};
    return &renderer;
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

  void Renderer::DrawAxisIndicator(std::span<RenderView> renderViews)
  {
    GFX::DebugMarker marker("Draw axis indicator");

    glDepthFunc(GL_ALWAYS); // allows indicator to always be rendered
    glBlendFunc(GL_ONE, GL_ZERO);
    glLineWidth(2.f);
    glBindVertexArray(axisVao);

    auto currShader = GFX::ShaderManager::Get()->GetShader("axis");
    currShader->Bind();

    for (auto& renderView : renderViews)
    {
      if (!(renderView.mask & GFX::RenderMaskBit::RenderScreenElements))
        continue;

      glViewport(renderView.offset.x, renderView.offset.y, renderView.size.width, renderView.size.height);

      renderView.renderTarget->Bind();
      const Camera& c = *renderView.camera;
      currShader->SetMat4("u_model", glm::translate(glm::mat4(1), c.viewInfo.position + c.viewInfo.GetForwardDir() * 10.f)); // add scaling factor (larger # = smaller visual)
      currShader->SetMat4("u_view", c.viewInfo.GetViewMatrix());
      currShader->SetMat4("u_proj", c.proj);
      glDrawArrays(GL_LINES, 0, 6);
    }

    glDepthFunc(GL_GEQUAL);
    glBindVertexArray(0);
  }

  void Renderer::DrawSkybox(std::span<RenderView> renderViews)
  {
    GFX::DebugMarker marker("Draw skybox");

    //glDepthMask(GL_FALSE);
    glDepthFunc(GL_EQUAL);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(emptyVao);

    auto shdr = GFX::ShaderManager::Get()->GetShader("skybox");
    shdr->Bind();
    shdr->SetInt("u_skybox", 0);
    env.skyboxView->Bind(0, *env.skyboxSampler);

    for (auto& renderView : renderViews)
    {
      if (!(renderView.mask & GFX::RenderMaskBit::RenderSky))
        continue;

      glViewport(renderView.offset.x, renderView.offset.y, renderView.size.width, renderView.size.height);

      renderView.renderTarget->Bind();
      const auto& c = *renderView.camera;
      shdr->SetMat4("u_proj", c.proj);
      shdr->SetMat4("u_modview", glm::translate(c.viewInfo.GetViewMatrix(), c.viewInfo.position));
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
      glTextureBarrier();
    }

    //glDepthMask(GL_TRUE);
    glDepthFunc(GL_GEQUAL);
    glEnable(GL_CULL_FACE);
    env.skyboxView->Unbind(0);
  }

  void Renderer::StartFrame()
  {
    GL_ResetState();

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

  void GFX::Renderer::ClearFramebuffers(std::span<RenderView> renderViews)
  {
    for (auto& renderView : renderViews)
    {
      renderView.renderTarget->Bind();
      if (renderView.mask & RenderMaskBit::ClearColorEachFrame)
        glClear(GL_COLOR_BUFFER_BIT);
      if (renderView.mask & RenderMaskBit::ClearDepthEachFrame)
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    GL_ResetState();
  }

  void Renderer::EndFrame(float dt)
  {
    GFX::DebugMarker endframeMarker("Postprocessing");
    MEASURE_GPU_TIMER_STAT(Postprocessing);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ldrFbo->Bind();

    {
      GFX::DebugMarker tonemappingMarker("Tone mapping");

      const float logLowLum = glm::log(tonemap.targetLuminance / tonemap.maxExposure);
      const float logMaxLum = glm::log(tonemap.targetLuminance / tonemap.minExposure);
      const int computePixelsX = glm::ceil(GetRenderWidth() / 4.0f);
      const int computePixelsY = glm::ceil(GetRenderHeight() / 4.0f);

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
        hdrColorTexView->Bind(1, *defaultSampler);
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
      hdrColorTexView->Bind(1, *defaultSampler);
      tonemap.blueNoiseView->Bind(2, *tonemap.blueNoiseSampler);
      glBindVertexArray(emptyVao);
      //glBindTextureUnit(1, fog.tex); // rebind because AMD drivers sus
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glDepthMask(GL_TRUE);
      glEnable(GL_CULL_FACE);
      tonemap.blueNoiseView->Unbind(2);
      hdrColorTexView->Unbind(1);
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

  GLFWwindow* const* Renderer::Init()
  {
    window_ = InitContext();

    InitVertexBuffers();
    InitFramebuffers();
    InitVertexLayouts();
    CompileShaders();
    InitTextures();

    std::vector<int> zeros(tonemap.NUM_BUCKETS, 0);
    tonemap.exposureBuffer = std::make_unique<GFX::StaticBuffer>(zeros.data(), 2 * sizeof(float));
    tonemap.histogramBuffer = std::make_unique<GFX::StaticBuffer>(zeros.data(), tonemap.NUM_BUCKETS * sizeof(int));

    // enable debugging stuff
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLerrorCB, NULL);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    GL_ResetState();

    Console::Get()->RegisterCommand("showShaders", "- Lists all shader names", logShaderNames);
    Console::Get()->RegisterCommand("recompile", "- Recompiles a named shader", recompileShader);
    vsyncCvar.Set(0);

    return &window_;
  }

  void Renderer::InitFramebuffers()
  {
    //const int levels = glm::floor(glm::log2(glm::max(fboWidth, fboHeight))) + 1;
    TextureCreateInfo hdrColorTexInfo
    {
      .imageType = ImageType::TEX_2D,
      .format = Format::R16G16B16A16_FLOAT,
      .extent = Extent3D{ .width = GetRenderWidth(), .height = GetRenderHeight(), .depth = 1 },
      .mipLevels = 1,
      .arrayLayers = 1
    };
    TextureCreateInfo ldrColorTexInfo
    {
      .imageType = ImageType::TEX_2D,
      .format = Format::R8G8B8A8_UNORM,
      //.format = Format::R16G16B16A16_UNORM,
      .extent { .width = GetRenderWidth(), .height = GetRenderHeight(), .depth = 1 },
      .mipLevels = 1,
      .arrayLayers = 1
    };
    TextureCreateInfo depthTexInfo
    {
      .imageType = ImageType::TEX_2D,
      .format = Format::D32_FLOAT,
      .extent = Extent3D{ .width = GetRenderWidth(), .height = GetRenderHeight(), .depth = 1 },
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

    defaultSampler.emplace(*TextureSampler::Create({}, "Plain Sampler"));

    glViewport(0, 0, GetRenderWidth(), GetRenderHeight());
  }

  void Renderer::InitVertexBuffers()
  {
    // TODO: use dynamically sized buffer
    vertexBuffer = std::make_unique<GFX::DynamicBuffer<>>(100'000'000, sizeof(Vertex));
    indexBuffer = std::make_unique<GFX::DynamicBuffer<>>(100'000'000, sizeof(GLuint));

    constexpr float indicatorVertices[] =
    {
      // positions      // colors
      0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // x-axis
      1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // y-axis
      0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // z-axis
      0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };
    axisVbo.emplace(indicatorVertices, sizeof(indicatorVertices));
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

    // empty VAO for bufferless drawing
    glCreateVertexArrays(1, &emptyVao);

    // VAO for rendering the axis indicator
    glCreateVertexArrays(1, &axisVao);
    glEnableVertexArrayAttrib(axisVao, 0); // pos
    glEnableVertexArrayAttrib(axisVao, 1); // color
    glVertexArrayAttribBinding(axisVao, 0, 0);
    glVertexArrayAttribBinding(axisVao, 1, 0);
    glVertexArrayAttribFormat(axisVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(axisVao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayVertexBuffer(axisVao, 0, axisVbo->GetID(), 0, 6 * sizeof(float));
  }

  void Renderer::InitTextures()
  {
    TextureManager::Get()->AddTexture("blueNoiseRGB",
      *LoadTexture2D("BlueNoise/16_16/LDR_RGB1_0.png", GFX::Format::R8G8B8A8_UNORM));
    tonemap.blueNoiseView = GFX::TextureView::Create(*TextureManager::Get()->GetTexture("blueNoiseRGB"), "BlueNoiseRGBView");
    SamplerState samplerState{};
    samplerState.asBitField.addressModeU = AddressMode::REPEAT;
    samplerState.asBitField.addressModeV = AddressMode::REPEAT;
    tonemap.blueNoiseSampler.emplace(*TextureSampler::Create(samplerState, "BlueNoiseRGBSampler"));

    const std::string_view faces[6] =
    {
      "autumn_sky_hdr/px.hdr",
      "autumn_sky_hdr/nx.hdr",
      "autumn_sky_hdr/py.hdr",
      "autumn_sky_hdr/ny.hdr",
      "autumn_sky_hdr/pz.hdr",
      "autumn_sky_hdr/nz.hdr",
    };
    env.skyboxMemory = GFX::LoadTextureCube(faces);
    env.skyboxView = TextureView::Create(*env.skyboxMemory, "skycube view");
    SamplerState cubesamplerState{};
    cubesamplerState.asBitField.addressModeU = AddressMode::MIRRORED_REPEAT;
    cubesamplerState.asBitField.addressModeV = AddressMode::MIRRORED_REPEAT;
    cubesamplerState.asBitField.addressModeW = AddressMode::MIRRORED_REPEAT;
    env.skyboxSampler = TextureSampler::Create(cubesamplerState, "skycube sampler");
  }

  void Renderer::GL_ResetState()
  {
    // texture unit and sampler bindings (first 8, hopefully more than we'll ever need)
    for (int i = 0; i < 8; i++)
    {
      glBindSampler(i, 0);
      glBindTextureUnit(i, 0);
    }
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // triangle winding
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // depth test
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_GEQUAL);

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
    glClearDepth(0.0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    hdrFbo->Bind();
  }

  GLFWwindow* Renderer::InitContext()
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
    // set the window dims here, although it won't necessarily be what we get
    windowWidth = videoMode->width;
    windowHeight = videoMode->height;
    CreateWindow(isFullscreen);

    if (!window_)
    {
      spdlog::critical("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window_);
    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow*, int width, int height)
      {
        Renderer::Get()->SetFramebufferSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
      });

    if (!gladLoadGL())
    {
      spdlog::critical("Failed to initialize GLAD");
    }

    return window_;
  }

  // call when the window needs to be recreated, like when full screen mode is toggled
  GLFWwindow* Renderer::CreateWindow(bool fullscreen)
  {
    if (window_)
    {
      glfwDestroyWindow(window_);
    }

    isFullscreen = fullscreen;

    if (fullscreen)
    {
      GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
      const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
      window_ = glfwCreateWindow(videoMode->width, videoMode->height, "Gengine", primaryMonitor, nullptr);
    }
    else
    {
      window_ = glfwCreateWindow(windowWidth, windowHeight, "Gengine", nullptr, nullptr);
    }

    // set the window dims to what we actually got
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    windowWidth = width;
    windowHeight = height;

    renderWidth = glm::max(static_cast<uint32_t>(windowWidth * renderScale), 1u);
    renderHeight = glm::max(static_cast<uint32_t>(windowHeight * renderScale), 1u);
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
    renderWidth = glm::max(static_cast<uint32_t>(windowWidth * renderScale), 1u);
    renderHeight = glm::max(static_cast<uint32_t>(windowHeight * renderScale), 1u);
    InitFramebuffers();
  }
}