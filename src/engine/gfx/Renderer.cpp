#include "../PCH.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/norm.hpp>
#include "Renderer.h"

#include "ShaderManager.h"
#include "Mesh.h"
#include "api/DebugMarker.h"

#include <engine/ecs/system/ParticleSystem.h>
#include "api/Buffer.h"

#include <engine/ecs/component/Transform.h>
#include <engine/ecs/component/Rendering.h>
#include <engine/ecs/component/ParticleEmitter.h>
#include "TextureManager.h"
#include "TextureLoader.h"
#include "api/Fence.h"

#include <execution>
#include <iostream>
#include <vector>
#include <array>
#include "../CVar.h"
#include "../Console.h"
#include "../Parser.h"
#include <engine/core/StatMacros.h>
#include "../../utility/MathExtensions.h"
#include "Material.h"
#include "api/Texture.h"
#include "api/Framebuffer.h"
#include "api/LinearBufferAllocator.h"

#include "fx/FXAA.h"
#include "fx/Bloom.h"
#include "fx/CubemapReflections.h"
#include "fx/Fog.h"

#include <imgui/imgui.h>

#define LOG_PARTICLE_RENDER_TIME 0

DECLARE_FLOAT_STAT(LuminanceHistogram, GPU)
DECLARE_FLOAT_STAT(CameraExposure, GPU)
DECLARE_FLOAT_STAT(FXAA, GPU)
DECLARE_FLOAT_STAT(ReflectionsTrace, GPU)
DECLARE_FLOAT_STAT(ReflectionsSample, GPU)
DECLARE_FLOAT_STAT(ReflectionsDenoise, GPU)
DECLARE_FLOAT_STAT(ReflectionsComposite, GPU)
DECLARE_FLOAT_STAT(Bloom_GPU, GPU)
DECLARE_FLOAT_STAT(Bloom_CPU, CPU)

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
    if (fullscreen == Renderer::GetIsFullscreen())
      return;
    Renderer::CreateWindow(fullscreen);
    Renderer::InitFramebuffers();
  }

  void setRenderScale([[maybe_unused]] const char* cvar, cvar_float scale)
  {
    Renderer::SetRenderingScale(static_cast<float>(scale));
  }

  void logShaderNames(const char*)
  {
    auto shaderNames = ShaderManager::GetAllShaderNames();
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
    if (!ShaderManager::GetShader(id))
    {
      Console::Get()->Log("No shader found with name %s", str->c_str());
      return;
    }

    if (!ShaderManager::RecompileShader(id))
    {
      Console::Get()->Log("Failed to recompile shader %s", str->c_str());
      return;
    }
  }

  void setReflectionScale([[maybe_unused]] const char* cvar, cvar_float scale)
  {
    Renderer::SetReflectionsRenderScale(static_cast<float>(scale));
  }

  void setReflectionsModeCallback(const char*, cvar_float scale)
  {
    Renderer::SetUpdateProbes(scale < Renderer::REFLECTION_MODE_CUBE_THRESHOLD);
  }

  AutoCVar<cvar_float> vsyncCvar("r.vsync", "- Whether vertical sync is enabled", 0, 0, 1, CVarFlag::NONE, vsyncCallback);
  AutoCVar<cvar_float> renderScaleCvar("r.scale", "- Internal rendering resolution scale", 1.0, 0.1, 2.0, CVarFlag::NONE, setRenderScale);
  AutoCVar<cvar_float> reflectionsScaleCvar("r.reflections.scale", "- Internal reflections resolution scale", 1.0, 0.1, 1.0, CVarFlag::NONE, setReflectionScale);
  AutoCVar<cvar_float> reflectionsModeCvar("r.reflections.mode", "- Reflections mode. 0: skybox, 1: probe, 2: parallax correct probe", 2.0, 0.0, 2.0);
  AutoCVar<cvar_float> reflectionsDenoiseEnableCvar("r.reflections.denoiseEnable", "- If true, reflections will receive spatial denoising", 1.0, 0.0, 1.0);
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

  namespace Renderer
  {
    namespace
    {
      GLFWwindow* window_{};
      bool isFullscreen{ false };

      std::vector<std::string> openglExtensions;


      // particle rendering
      struct EmitterDrawCommand
      {
        const Component::ParticleEmitter* emitter;
        glm::mat4 modelUniform;
      };
      std::atomic_uint32_t emitterDrawIndex = 0;
      std::vector<EmitterDrawCommand> emitterDrawCommands;

      // std140
      struct UniformData
      {
        glm::mat4 model;
      };

      // batched+instanced rendering stuff (ONE MATERIAL SUPPORTED ATM)
      std::optional<Buffer> vertexBuffer;
      std::optional<Buffer> indexBuffer;
      std::optional<LinearBufferAllocator> vertexBufferAlloc;
      std::optional<LinearBufferAllocator> indexBufferAlloc;

      // per-vertex layout
      uint32_t batchVAO{};

      // maps handles to VERTEX and INDEX information in the respective dynamic buffers
      // used to retrieve important offset and size info for meshes
      std::map<MeshID, DrawElementsIndirectCommand> meshBufferInfo;

      struct BatchDrawCommand
      {
        MeshID mesh;
        MaterialID material;
        glm::mat4 modelUniform;
      };
      std::vector<BatchDrawCommand> userCommands;
      std::atomic_uint32_t cmdIndex{ 0 };

      uint32_t emptyVao{};

      uint32_t axisVao{};
      std::optional<Buffer> axisVbo;

      std::optional<TextureSampler> defaultSampler;
      std::optional<TextureSampler> nearestSampler;
      std::optional<TextureSampler> linearMipmapNearestSampler;

      std::optional<Texture> ldrColorTexMemory;
      std::optional<TextureView> ldrColorTexView;
      std::optional<Texture> postAATexMemory;
      std::optional<TextureView> postAATexView;
      uint32_t windowWidth{ 1 };
      uint32_t windowHeight{ 1 };
      uint32_t renderWidth{ 1 };
      uint32_t renderHeight{ 1 };
      float renderScale{ 1.0f }; // 1.0 means render resolution will match window

      struct GBuffer
      {
        std::optional<Framebuffer> fbo;
        RenderView renderView{};
        Camera camera{};
        std::optional<Texture> colorTexMemory;
        std::optional<Texture> normalTexMemory;
        std::optional<Texture> depthTexMemory;
        std::optional<Texture> PBRTexMemory;
        std::optional<TextureView> colorTexView;
        std::optional<TextureView> normalTexView;
        std::optional<TextureView> depthTexView;
        std::optional<TextureView> PBRTexView;
      }gBuffer;

      std::optional<TextureView> blueNoiseRView;

      std::optional<TextureView> blueNoiseBigView;

      struct Reflections_t
      {
        std::optional<Framebuffer> fbo;
        std::optional<Texture> texMemory[2];
        std::optional<TextureView> texView[2];
        std::optional<TextureSampler> scratchSampler;
        std::optional<TextureSampler> scratchSampler2;
        float renderScale{};
        Extent2D fboSize{};

        struct
        {
          std::array<float, 5> kernel = { 0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f };
          std::array<float, 5> offsets = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };
          uint32_t num_passes{ 1 };
          float n_phi{ 1 };
          float p_phi{ 1 };
          float step_width{ 1.0f };
        }atrous;
      }reflect;

      struct ProbeData_t
      {
        std::array<RenderView, 6> renderViews;
        std::array<Camera, 6> cameras;
        std::array<std::optional<TextureView>, 6> colorViews;
        std::array<std::optional<TextureView>, 6> depthViews;
        std::array<std::optional<TextureView>, 6> distanceViews;
        std::optional<Texture> colorCube;
        std::optional<Texture> depthCube;
        std::optional<Texture> distanceCube;
        std::optional<TextureView> colorCubeView;
        std::optional<TextureView> distanceCubeView;
        Format colorFormat = Format::R11G11B10_FLOAT;
        Format depthFormat = Format::D16_UNORM;
        Format distanceFormat = Format::R16_FLOAT;
        Extent2D imageSize{ 512, 512 };
      }probeData;

      struct Composited_t
      {
        // post-shading
        std::optional<Framebuffer> fbo;
      }composited;

      struct Environment_t
      {
        std::optional<Texture> skyboxMemory;
        std::optional<TextureView> skyboxView;
        std::optional<TextureSampler> skyboxSampler;
      }env;

      struct FogParams_t
      {
        glm::vec3 albedo{ 1.0 };
        float u_a = 0.005f;
        float u_b = 10000.0f;
        float u_heightOffset = -40.0f;
        float u_fog2Density = 0.005f;
        float u_beer = 1.0f;
        float u_powder = 1.0f;
      }fog;

      struct TonemapperParams_t
      {
        float exposure = 1.0f;
        float minExposure = 0.1f;
        float maxExposure = 10.0f;
        float targetLuminance = .22f;
        float adjustmentSpeed = 0.5f;
        bool gammaCorrection = true;
        std::optional<Buffer> exposureBuffer;
        std::optional<Buffer> histogramBuffer;
        const int NUM_BUCKETS = 128;
        std::optional<TextureView> blueNoiseView;
        std::optional<TextureSampler> blueNoiseSampler;
        bool tonemapDither = true;
      }tonemap;

      struct Bloom_t
      {
        bool enabled{ true };
        float strength{ 1.0f / 64.0f };
        float width{ 1.0f };
        uint32_t passes{ 6 };
        std::optional<Texture> scratchTex;
        std::optional<TextureView> scratchTexView;
        std::optional<TextureSampler> scratchSampler;
      }bloom;

      struct FXAAParams_t
      {
        bool enabled{ true };
        float contrastThreshold = 0.0312;
        float relativeThreshold = 0.125;
        float pixelBlendStrength = 0.2;
        float edgeBlendStrength = 1.0;
        std::optional<TextureSampler> scratchSampler;
      }fxaa;
    }

    GLFWwindow* InitContext()
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

      //CreateWindow(true);
      CreateWindow(isFullscreen);

      if (!window_)
      {
        spdlog::critical("Failed to create GLFW window");
      }

      glfwMakeContextCurrent(window_);
      glfwSetFramebufferSizeCallback(window_, [](GLFWwindow*, int width, int height)
        {
          Renderer::SetFramebufferSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        });

      // load OpenGL function pointers
      if (!gladLoadGL())
      {
        spdlog::critical("Failed to initialize GLAD");
      }

      // query all available extensions
      openglExtensions.clear();
      GLint extensionCount = 0;
      glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
      for (int i = 0; i < extensionCount; i++)
      {
        // spoopy
        openglExtensions.push_back(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)));
      }

      Console::Get()->Log("%zu OpenGL extensions supported", openglExtensions.size());
      //for (const auto& extension : openglExtensions)
      //{
      //  Console::Get()->Log("%s", extension.c_str());
      //}

      return window_;
    }

    uint32_t GetRenderWidth()
    {
      return renderWidth;
    }

    uint32_t GetRenderHeight()
    {
      return renderHeight;
    }

    void GL_ResetState()
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
      gBuffer.fbo->Bind();
    }

    void AddBatchedMesh(MeshID id, const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
    {
      auto vOffset = vertexBufferAlloc->Allocate(std::span(vertices), sizeof(Vertex));
      auto iOffset = indexBufferAlloc->Allocate(std::span(indices), sizeof(Index));
      // generate an indirect draw command with most of the info needed to draw this mesh
      DrawElementsIndirectCommand cmd{};
      cmd.baseVertex = vOffset / sizeof(Vertex);
      cmd.instanceCount = 0;
      cmd.count = static_cast<uint32_t>(indices.size());
      cmd.firstIndex = iOffset / sizeof(Index);
      //cmd.baseInstance = ?; // only knowable after all user draw calls are submitted
      meshBufferInfo[id] = cmd;
    }

    float GetWindowAspectRatio()
    {
      return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    }

    Extent2D GetWindowDimensions()
    {
      return { renderWidth, renderHeight };
    }

    RenderView* GetProbeRenderView(size_t index)
    {
      ASSERT(index < 6); return &probeData.renderViews[index];
    }

    RenderView* GetMainRenderView()
    {
      return &gBuffer.renderView;
    }

    bool GetIsFullscreen()
    {
      return isFullscreen;
    }

    void BeginObjects(size_t maxDraws)
    {
      cmdIndex = 0;
      userCommands.resize(maxDraws);
    }

    void SubmitObject(const Component::Model& model, const Component::BatchedMesh& mesh, const Component::Material& mat)
    {
      auto index = cmdIndex.fetch_add(1, std::memory_order::memory_order_acq_rel);
      userCommands[index] = BatchDrawCommand{ .mesh = mesh.handle, .material = mat.handle, .modelUniform = model.matrix };
    }

    void RenderBatchHelper(std::span<RenderView*> renderViews, MaterialID mat, const std::vector<UniformData>& uniforms)
    {
      //ASSERT(MaterialManager::Get()->materials_.contains(mat));
      auto material = *MaterialManager::GetMaterialInfo(mat);
      DebugMarker marker(("Batch: " + std::string(material.shaderID)).c_str());

      // generate SSBO w/ uniforms
      //Buffer uniformBuffer = Buffer(uniforms.data(), uniforms.size() * sizeof(UniformData));
      auto uniformBuffer = Buffer::Create(std::span(uniforms));
      uniformBuffer->Bind<Target::SHADER_STORAGE_BUFFER>(0);

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
      //Buffer dib(commands.data(), commands.size() * sizeof(DrawElementsIndirectCommand));
      auto drawIndirectBuffer = Buffer::Create(std::span(commands));
      drawIndirectBuffer->Bind<Target::DRAW_INDIRECT_BUFFER>();

      // clear instance count for next GL draw command
      for (auto& info : meshBufferInfo)
      {
        info.second.instanceCount = 0;
      }

      // do the actual draw
      auto shader = ShaderManager::GetShader(material.shaderID);
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
        BindTextureView(i++, view, sampler);
      }

      // TODO: perform GPU culling somewhere around here

      auto framebuffer = Framebuffer::Create();
      framebuffer->Bind();

      for (auto& renderView : renderViews)
      {
        if (!(renderView->mask & RenderMaskBit::RenderObjects))
          continue;

        SetFramebufferDrawBuffersAuto(*framebuffer, renderView->renderInfo, 3);

        ASSERT(renderView->renderInfo.depthAttachment.has_value());
        framebuffer->SetAttachment(Attachment::DEPTH, *renderView->renderInfo.depthAttachment->textureView, 0);

        shader->SetMat4("u_viewProj", renderView->camera->GetViewProj());

        SetViewport(renderView->renderInfo);
        glBindVertexArray(batchVAO);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0, static_cast<GLsizei>(commands.size()), 0);
      }

      for (int i = 0; auto & [view, sampler] : material.viewSamplers)
      {
        UnbindTextureView(i++);
      }
    }

    void RenderObjects(std::span<RenderView*> renderViews)
    {
      DebugMarker marker("Draw batched objects");

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

    void BeginEmitters(size_t maxDraws)
    {
      emitterDrawIndex = 0;
      emitterDrawCommands.resize(maxDraws);
    }

    void SubmitEmitter(const Component::ParticleEmitter& emitter, const Component::Transform& model)
    {
      auto index = emitterDrawIndex.fetch_add(1);
      emitterDrawCommands[index] = { &emitter, model.GetModel() };
    }

    void RenderEmitters(std::span<RenderView*> renderViews)
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
      auto shader = ShaderManager::GetShader("particle");
      shader->Bind();

      auto framebuffer = Framebuffer::Create();
      framebuffer->SetDrawBuffers({ { Attachment::COLOR_0 } });
      framebuffer->Bind();

      for (auto& renderView : renderViews)
      {
        if (!(renderView->mask & RenderMaskBit::RenderEmitters))
          continue;

        SetViewport(renderView->renderInfo);
        ASSERT(renderView->renderInfo.colorAttachments[0].has_value());
        ASSERT(renderView->renderInfo.depthAttachment.has_value());
        framebuffer->SetAttachment(Attachment::COLOR_0, *renderView->renderInfo.colorAttachments[0]->textureView, 0);
        framebuffer->SetAttachment(Attachment::DEPTH, *renderView->renderInfo.depthAttachment->textureView, 0);

        auto compare = [&renderView](const EmitterDrawCommand& p1, EmitterDrawCommand& p2)
        {
          auto pos1 = glm::vec3(p1.modelUniform[3]);
          auto pos2 = glm::vec3(p2.modelUniform[3]);
          if (pos1 != pos2)
          {
            auto len = glm::length2(pos1 - renderView->camera->viewInfo.position) -
              glm::length2(pos2 - renderView->camera->viewInfo.position);
            if (glm::abs(len) > 0.001f)
            {
              return len > 0.0f;
            }
          }

          return p1.emitter < p2.emitter;
        };

        std::sort(emitterDrawCommands.begin(), emitterDrawCommands.end(), compare);

        auto v = renderView->camera->viewInfo.GetViewMatrix();
        shader->SetMat4("u_viewProj", renderView->camera->GetViewProj());
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

    void DrawFog(std::span<RenderView*> renderViews, bool earlyFogPass)
    {
      DebugMarker fogMarker("Fog");

      for (auto& renderView : renderViews)
      {
        if (!(renderView->mask & RenderMaskBit::RenderFog || renderView->mask & RenderMaskBit::RenderEarlyFog))
          continue;
        if (earlyFogPass && !(renderView->mask & RenderMaskBit::RenderEarlyFog))
          continue;
        if (!earlyFogPass && (renderView->mask & RenderMaskBit::RenderEarlyFog))
          continue;

        ASSERT(renderView->renderInfo.colorAttachments[0].has_value());
        ASSERT(renderView->renderInfo.depthAttachment.has_value());

        FX::FogParameters params
        {
          .sourceColor = *renderView->renderInfo.colorAttachments[0]->textureView,
          .sourceDepth = *renderView->renderInfo.depthAttachment->textureView,
          .targetColor = *renderView->renderInfo.colorAttachments[0]->textureView,
          .scratchSampler = *reflect.scratchSampler,
          .camera = gBuffer.camera,
          .a = fog.u_a,
          .b = fog.u_b,
          .heightOffset = fog.u_heightOffset,
          .fog2Density = fog.u_fog2Density,
          .albedo = fog.albedo,
          .beer = fog.u_beer,
          .powder = fog.u_powder
        };

        FX::ApplyFog(params);
      }
    }

    void CompileShaders()
    {
      ShaderManager::AddShader("batched",
        {
          { "TexturedMeshBatched.vs.glsl", ShaderType::VERTEX },
          { "TexturedMesh.fs.glsl", ShaderType::FRAGMENT }
        });

      ShaderManager::AddShader("chunk_optimized",
        {
          { "chunk_optimized.vs.glsl", ShaderType::VERTEX },
          { "chunk_optimized.fs.glsl", ShaderType::FRAGMENT }
        });
      ShaderManager::AddShader("compact_batch",
        { { "compact_batch.cs.glsl", ShaderType::COMPUTE } });
      ShaderManager::AddShader("update_particle_emitter",
        { { "update_particle_emitter.cs.glsl", ShaderType::COMPUTE } });
      ShaderManager::AddShader("update_particle",
        { { "update_particle.cs.glsl", ShaderType::COMPUTE } });
      ShaderManager::AddShader("textured_array",
        {
          { "textured_array.vs.glsl", ShaderType::VERTEX },
          { "textured_array.fs.glsl", ShaderType::FRAGMENT }
        });
      ShaderManager::AddShader("buffer_vis",
        {
          { "buffer_vis.fs.glsl", ShaderType::FRAGMENT },
          { "buffer_vis.vs.glsl", ShaderType::VERTEX }
        });
      ShaderManager::AddShader("chunk_render_cull",
        {
          { "chunk_render_cull.vs.glsl", ShaderType::VERTEX },
          { "chunk_render_cull.fs.glsl", ShaderType::FRAGMENT }
        });
      ShaderManager::AddShader("particle",
        {
          { "particle.vs.glsl", ShaderType::VERTEX },
          { "particle.fs.glsl", ShaderType::FRAGMENT }
        });
      ShaderManager::AddShader("skybox",
        {
          { "skybox.vs.glsl", ShaderType::VERTEX },
          { "skybox.fs.glsl", ShaderType::FRAGMENT }
        });
      ShaderManager::AddShader("tonemap",
        {
          { "fullscreen_tri.vs.glsl", ShaderType::VERTEX },
          { "tonemap.fs.glsl", ShaderType::FRAGMENT }
        });
      ShaderManager::AddShader("calc_exposure",
        { { "calc_exposure.cs.glsl", ShaderType::COMPUTE } });
      ShaderManager::AddShader("generate_histogram",
        { { "generate_histogram.cs.glsl", ShaderType::COMPUTE } });

      //ShaderManager::AddShader("sun",
      //  {
      //    { "flat_sun.vs.glsl", ShaderType::VERTEX },
      //    { "flat_sun.fs.glsl", ShaderType::FRAGMENT }
      //  });
      ShaderManager::AddShader("axis",
        {
          { "axis.vs.glsl", ShaderType::VERTEX },
          { "axis.fs.glsl", ShaderType::FRAGMENT }
        });
      //ShaderManager::AddShader("flat_color",
      //  {
      //    { "flat_color.vs.glsl", ShaderType::VERTEX },
      //    { "flat_color.fs.glsl", ShaderType::FRAGMENT }
      //  });
      FX::CompileFXAAShader();
      FX::CompileReflectionShaders();
      FX::CompileBloomShaders();
      FX::CompileFogShader();
    }

    void DrawAxisIndicator(std::span<RenderView*> renderViews)
    {
      DebugMarker marker("Draw axis indicator");

      glDepthFunc(GL_ALWAYS); // allows indicator to always be rendered
      glBlendFunc(GL_ONE, GL_ZERO);
      glLineWidth(2.f);
      glBindVertexArray(axisVao);

      auto currShader = ShaderManager::GetShader("axis");
      currShader->Bind();

      auto framebuffer = Framebuffer::Create();
      framebuffer->SetDrawBuffers({ { Attachment::COLOR_0 } });
      framebuffer->Bind();

      for (auto& renderView : renderViews)
      {
        if (!(renderView->mask & RenderMaskBit::RenderScreenElements))
          continue;

        SetViewport(renderView->renderInfo);
        ASSERT(renderView->renderInfo.colorAttachments[0].has_value());
        framebuffer->SetAttachment(Attachment::COLOR_0, *renderView->renderInfo.colorAttachments[0]->textureView, 0);

        const Camera& c = *renderView->camera;
        currShader->SetMat4("u_model", glm::translate(glm::mat4(1), c.viewInfo.position + c.viewInfo.GetForwardDir() * 10.f)); // add scaling factor (larger # = smaller visual)
        currShader->SetMat4("u_view", c.viewInfo.GetViewMatrix());
        currShader->SetMat4("u_proj", c.proj);
        glDrawArrays(GL_LINES, 0, 6);
      }

      glDepthFunc(GL_GEQUAL);
      glBindVertexArray(0);
    }

    void DrawSky(std::span<RenderView*> renderViews)
    {
      DebugMarker marker("Draw skybox");

      //glDepthMask(GL_FALSE);
      glDepthFunc(GL_EQUAL);
      glDisable(GL_CULL_FACE);
      glBindVertexArray(emptyVao);

      auto shdr = ShaderManager::GetShader("skybox");
      shdr->Bind();
      shdr->SetInt("u_skybox", 0);
      BindTextureView(0, *env.skyboxView, *env.skyboxSampler);

      auto framebuffer = Framebuffer::Create();
      framebuffer->SetDrawBuffers({ { Attachment::COLOR_0 } });
      framebuffer->Bind();

      for (auto& renderView : renderViews)
      {
        if (!(renderView->mask & RenderMaskBit::RenderSky))
          continue;

        SetViewport(renderView->renderInfo);
        ASSERT(renderView->renderInfo.colorAttachments[0].has_value());
        ASSERT(renderView->renderInfo.depthAttachment.has_value());
        framebuffer->SetAttachment(Attachment::COLOR_0, *renderView->renderInfo.colorAttachments[0]->textureView, 0);
        framebuffer->SetAttachment(Attachment::DEPTH, *renderView->renderInfo.depthAttachment->textureView, 0);

        const auto& c = *renderView->camera;
        shdr->SetMat4("u_proj", c.proj);
        shdr->SetMat4("u_modview", glm::translate(c.viewInfo.GetViewMatrix(), c.viewInfo.position));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
        glTextureBarrier();
      }

      //glDepthMask(GL_TRUE);
      glDepthFunc(GL_GEQUAL);
      glEnable(GL_CULL_FACE);
      UnbindTextureView(0);
    }

    void StartFrame()
    {
      glClearColor(0, 0, 0, 0);

      reflect.fbo->Bind();
      glClear(GL_COLOR_BUFFER_BIT);

      gBuffer.fbo->Bind();
      glClear(GL_COLOR_BUFFER_BIT);

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

    void ClearFramebuffers(std::span<RenderView*> renderViews)
    {
      auto framebuffer = Framebuffer::Create();
      framebuffer->SetDrawBuffers({ { Attachment::COLOR_0 } });
      framebuffer->Bind();

      for (auto& renderView : renderViews)
      {
        for (size_t i = 0; i < RenderInfo::maxColorAttachments; i++)
        {
          const auto& attachment = renderView->renderInfo.colorAttachments[i];
          if (attachment && attachment->clearEachFrame)
          {
            framebuffer->SetAttachment(Attachment::COLOR_0, *attachment->textureView, 0);
            ASSERT(framebuffer->IsValid());
            const auto& f = attachment->clearValue.color.f;
            glClearColor(f[0], f[1], f[2], f[3]);
            glClear(GL_COLOR_BUFFER_BIT);
          }
          framebuffer->ResetAttachment(Attachment::COLOR_0);
        }

        if (const auto& depthAttachment = renderView->renderInfo.depthAttachment; depthAttachment && depthAttachment->clearEachFrame)
        {
          framebuffer->SetAttachment(Attachment::DEPTH, *depthAttachment->textureView, 0);
          glClearNamedFramebufferfv(framebuffer->GetAPIHandle(), GL_DEPTH, 0, &depthAttachment->clearValue.depthStencil.depth);
          framebuffer->ResetAttachment(Attachment::DEPTH);
        }

        if (const auto& stencilAttachment = renderView->renderInfo.stencilAttachment; stencilAttachment && stencilAttachment->clearEachFrame)
        {
          framebuffer->SetAttachment(Attachment::DEPTH, *stencilAttachment->textureView, 0);
          glClearNamedFramebufferiv(framebuffer->GetAPIHandle(), GL_STENCIL, 0, &stencilAttachment->clearValue.depthStencil.stencil);
          framebuffer->ResetAttachment(Attachment::STENCIL);
        }
      }

      GL_ResetState();
    }

    void ApplyAntialiasing()
    {
      if (fxaa.enabled)
      {
        DebugMarker marker1("FXAA");
        MEASURE_GPU_TIMER_STAT(FXAA);
        FX::ApplyFXAA(*ldrColorTexView, *postAATexView, fxaa.contrastThreshold, fxaa.relativeThreshold,
          fxaa.pixelBlendStrength, fxaa.edgeBlendStrength, *fxaa.scratchSampler);
      }
      else
      {
        DebugMarker marker1("Blit");
        auto fbo1 = Framebuffer::Create();
        auto fbo2 = Framebuffer::Create();
        fbo1->SetAttachment(Attachment::COLOR_0, *ldrColorTexView, 0);
        fbo2->SetAttachment(Attachment::COLOR_0, *postAATexView, 0);
        auto ext1 = ldrColorTexView->Extent();
        auto ext2 = postAATexView->Extent();
        Framebuffer::Blit(*fbo1, *fbo2,
          { 0, 0 }, { ext1.width, ext1.height },
          { 0, 0 }, { ext2.width, ext2.height },
          AspectMaskBit::COLOR_BUFFER_BIT, Filter::LINEAR);
      }
    }

    void WriteSwapchain()
    {
      DebugMarker marker0("Write to swapchain");

      auto framebuffer = Framebuffer::Create();
      framebuffer->SetAttachment(Attachment::COLOR_0, *postAATexView, 0);
      glBlitNamedFramebuffer(framebuffer->GetAPIHandle(), 0,
        0, 0, GetRenderWidth(), GetRenderHeight(),
        0, 0, windowWidth, windowHeight,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }

    void Bloom()
    {
      ImGui::Checkbox("Enable bloom", &bloom.enabled);
      ImGui::SliderFloat("Bloom width", &bloom.width, 0.1f, 5.0f);
      ImGui::SliderFloat("Bloom strength", &bloom.strength, 0.01f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
      ImGui::SliderInt("Bloom passes", (int*)&bloom.passes, 1, 7);

      if (bloom.enabled)
      {
        DebugMarker bloomMarker("Bloom");
        MEASURE_GPU_TIMER_STAT(Bloom_GPU);
        MEASURE_CPU_TIMER_STAT(Bloom_CPU);
        FX::ApplyBloom(*gBuffer.colorTexView, bloom.passes, bloom.strength, bloom.width,
          *bloom.scratchTexView, *bloom.scratchSampler);
      }
    }

    void ApplyTonemap(float dt)
    {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      auto framebuffer = Framebuffer::Create();
      framebuffer->SetAttachment(Attachment::COLOR_0, *ldrColorTexView, 0);
      framebuffer->Bind();

      {
        DebugMarker tonemappingMarker("Tone mapping");

        const float logLowLum = glm::log(tonemap.targetLuminance / tonemap.maxExposure);
        const float logMaxLum = glm::log(tonemap.targetLuminance / tonemap.minExposure);
        const int computePixelsX = glm::ceil(GetRenderWidth() / 4.0f);
        const int computePixelsY = glm::ceil(GetRenderHeight() / 4.0f);

        {
          //TimerQuery timerQuery;
          DebugMarker marker("Generate luminance histogram");
          MEASURE_GPU_TIMER_STAT(LuminanceHistogram);
          auto hshdr = ShaderManager::GetShader("generate_histogram");
          hshdr->Bind();
          hshdr->SetInt("u_hdrBuffer", 1);
          hshdr->SetFloat("u_logLowLum", logLowLum);
          hshdr->SetFloat("u_logMaxLum", logMaxLum);
          const int X_SIZE = 16;
          const int Y_SIZE = 8;
          int xgroups = (computePixelsX + X_SIZE - 1) / X_SIZE;
          int ygroups = (computePixelsY + Y_SIZE - 1) / Y_SIZE;
          tonemap.histogramBuffer->Bind<Target::SHADER_STORAGE_BUFFER>(0);
          BindTextureView(1, *gBuffer.colorTexView, *defaultSampler);
          //BindTextureView(1, *composited[frameNumber % 2].compositedTexView, *defaultSampler);
          glDispatchCompute(xgroups, ygroups, 1);
          glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
          //printf("Histogram time: %f ms\n", (double)timerQuery.Elapsed_ns() / 1000000.0);
        }

        //float expo{};
        //glGetNamedBufferSubData(exposureBuffer->GetID(), 0, sizeof(float), &expo);
        //printf("Exposure: %f\n", expo);

        {
          DebugMarker marker("Compute camera exposure");
          MEASURE_GPU_TIMER_STAT(CameraExposure);
          //glGenerateTextureMipmap(color);
          tonemap.exposureBuffer->Bind<Target::SHADER_STORAGE_BUFFER>(0);
          tonemap.histogramBuffer->Bind<Target::SHADER_STORAGE_BUFFER>(1);
          //floatBufferOut->Bind<Target::SSBO>(1);
          auto cshdr = ShaderManager::GetShader("calc_exposure");
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

        DebugMarker marker("Apply tone mapping");
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glViewport(0, 0, GetRenderWidth(), GetRenderHeight());
        auto shdr = ShaderManager::GetShader("tonemap");
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        shdr->Bind();
        shdr->SetFloat("u_exposureFactor", tonemap.exposure);
        shdr->SetBool("u_useDithering", tonemap.tonemapDither);
        shdr->SetBool("u_encodeSRGB", tonemap.gammaCorrection);
        BindTextureView(1, *gBuffer.colorTexView, *defaultSampler);
        //BindTextureView(1, *composited[frameNumber % 2].compositedTexView, *defaultSampler);
        BindTextureView(2, *tonemap.blueNoiseView, *tonemap.blueNoiseSampler);
        glBindVertexArray(emptyVao);
        //glBindTextureUnit(1, fog.tex); // rebind because AMD drivers sus
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        UnbindTextureView(2);
        UnbindTextureView(1);
      }

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glViewport(0, 0, windowWidth, windowHeight);
    }

    void InitFramebuffers()
    {
      Extent2D fboSize = { GetRenderWidth(), GetRenderHeight() };

      std::vector<int> zeros(tonemap.NUM_BUCKETS, 0);
      tonemap.exposureBuffer = Buffer::Create(std::span(zeros.data(), 2));
      tonemap.histogramBuffer = Buffer::Create(std::span(zeros));

      //const int levels = glm::floor(glm::log2(glm::max((float)fboSize.width, (float)fboSize.height))) + 1;

      gBuffer.colorTexMemory = CreateTexture2DMip(fboSize, Format::R16G16B16A16_FLOAT, 1, "GBuffer Color Texture");
      gBuffer.colorTexView = TextureView::Create(*gBuffer.colorTexMemory, "GBuffer Color View");
      gBuffer.depthTexMemory = CreateTexture2D(fboSize, Format::D32_FLOAT, "GBuffer Depth Texture");
      gBuffer.depthTexView = TextureView::Create(*gBuffer.depthTexMemory, "GBuffer Depth View");
      gBuffer.PBRTexMemory = CreateTexture2D(fboSize, Format::R8G8_UNORM, "GBuffer PBR Texture");
      gBuffer.PBRTexView = TextureView::Create(*gBuffer.PBRTexMemory, "GBuffer PBR View");
      gBuffer.normalTexMemory = CreateTexture2D(fboSize, Format::R16G16B16_SNORM, "GBuffer Normal Texture");
      gBuffer.normalTexView = TextureView::Create(*gBuffer.normalTexMemory, "GBuffer Normal View");
      gBuffer.fbo = Framebuffer::Create();
      gBuffer.fbo->SetAttachment(Attachment::COLOR_0, *gBuffer.colorTexView, 0);
      gBuffer.fbo->SetAttachment(Attachment::COLOR_1, *gBuffer.normalTexView, 0);
      gBuffer.fbo->SetAttachment(Attachment::COLOR_2, *gBuffer.PBRTexView, 0);
      gBuffer.fbo->SetAttachment(Attachment::DEPTH, *gBuffer.depthTexView, 0);
      gBuffer.fbo->SetDrawBuffers({ { Attachment::COLOR_0, Attachment::COLOR_1, Attachment::COLOR_2 } });
      ASSERT(gBuffer.fbo->IsValid());

      RenderAttachment diffuseAttachment
      {
        .textureView = &gBuffer.colorTexView.value(),
        .clearEachFrame = false
      };
      RenderAttachment normalAttachment
      {
        .textureView = &gBuffer.normalTexView.value(),
        .clearEachFrame = false
      };
      RenderAttachment pbrAttachment
      {
        .textureView = &gBuffer.PBRTexView.value(),
        .clearEachFrame = true,
        .clearValue = {.color = {.f = { 1, 0, 0, 0 } } }
      };
      RenderAttachment depthAttachment
      {
        .textureView = &gBuffer.depthTexView.value(),
        .clearEachFrame = true,
        .clearValue = {.depthStencil = {.depth = 0.0 } }
      };
      RenderInfo renderInfo
      {
        .offset = { 0, 0 },
        .size = fboSize,
        .colorAttachments = { diffuseAttachment, normalAttachment, pbrAttachment },
        .depthAttachment = { depthAttachment },
      };
      gBuffer.renderView.renderInfo = renderInfo;
      gBuffer.renderView.camera = &gBuffer.camera;

      ldrColorTexMemory = CreateTexture2D(fboSize, Format::R8G8B8A8_UNORM, "LDR Color Texture");
      ldrColorTexView = TextureView::Create(*ldrColorTexMemory, "LDR Color View");
      postAATexMemory = CreateTexture2D(fboSize, Format::R8G8B8A8_UNORM, "Post AA Texture");
      postAATexView = TextureView::Create(*postAATexMemory, "Post AA View");

      composited.fbo = Framebuffer::Create();
      composited.fbo->SetAttachment(Attachment::COLOR_0, *gBuffer.colorTexView, 0);
      ASSERT(composited.fbo->IsValid());

      bloom.scratchTex = CreateTexture2DMip({ fboSize.width / 2, fboSize.height / 2 }, Format::R11G11B10_FLOAT, bloom.passes);
      bloom.scratchTexView = bloom.scratchTex->View();
      bloom.scratchSampler = TextureSampler::Create({});

      fxaa.scratchSampler = TextureSampler::Create({});

      SamplerState ss{};
      ss.asBitField.minFilter = Filter::LINEAR;
      ss.asBitField.magFilter = Filter::LINEAR;
      ss.asBitField.mipmapFilter = Filter::LINEAR;
      ss.asBitField.addressModeU = AddressMode::MIRRORED_REPEAT;
      ss.asBitField.addressModeV = AddressMode::MIRRORED_REPEAT;
      ss.asBitField.addressModeW = AddressMode::MIRRORED_REPEAT;
      defaultSampler.emplace(*TextureSampler::Create(ss, "Default Sampler"));

      ss.asBitField.mipmapFilter = Filter::LINEAR;
      linearMipmapNearestSampler = TextureSampler::Create(ss, "Linear Mipmap Nearest Sampler");

      ss.asBitField.minFilter = Filter::NEAREST;
      ss.asBitField.magFilter = Filter::NEAREST;
      ss.asBitField.mipmapFilter = Filter::NEAREST;
      nearestSampler.emplace(*TextureSampler::Create(ss, "Nearest Sampler"));

      SetReflectionsRenderScale(0.5f);

      glViewport(0, 0, GetRenderWidth(), GetRenderHeight());
    }

    void InitReflectionFramebuffer()
    {
      for (int i = 0; i < 2; i++)
      {
        reflect.texMemory[i] = CreateTexture2D(reflect.fboSize, Format::R16G16B16A16_FLOAT, ("Reflection Texture " + std::to_string(i)).c_str());
        reflect.texView[i] = TextureView::Create(*reflect.texMemory[i], ("Reflection View " + std::to_string(i)).c_str());
      }
      reflect.fbo = Framebuffer::Create();
      reflect.fbo->SetAttachment(Attachment::COLOR_0, *reflect.texView[0], 0);
      reflect.fbo->SetDrawBuffers({ { Attachment::COLOR_0 } });
      reflect.scratchSampler = TextureSampler::Create({});
      reflect.scratchSampler2 = TextureSampler::Create({});
      ASSERT(reflect.fbo->IsValid());
    }

    bool QueryOpenGLExtensionStatus(std::string_view extensionName)
    {
      for (const auto& extension : openglExtensions)
      {
        if (extension == extensionName)
        {
          return true;
        }
      }
      return false;
    }

    const std::vector<std::string>& GetAllOpenGLExtensions()
    {
      return openglExtensions;
    }

    void SetProbePosition(glm::vec3 worldPos)
    {
      for (size_t i = 0; i < 6; i++)
      {
        probeData.cameras[i].viewInfo.position = worldPos;
      }
    }

    void SetProbeRenderMask(RenderMask mask)
    {
      for (size_t i = 0; i < 6; i++)
      {
        probeData.renderViews[i].mask = mask;
      }
    }

    // TODO: this function
    void SetUpdateProbes(bool b)
    {
      if (b)
      {

      }
      else
      {

      }
    }

    void InitVertexBuffers()
    {
      vertexBuffer = Buffer::Create(10'000'000, BufferFlag::DYNAMIC_STORAGE);
      indexBuffer = Buffer::Create(10'000'000, BufferFlag::DYNAMIC_STORAGE);
      vertexBufferAlloc = LinearBufferAllocator::Create(&vertexBuffer.value());
      indexBufferAlloc = LinearBufferAllocator::Create(&indexBuffer.value());

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
      axisVbo = Buffer::Create(std::span(indicatorVertices, sizeof(indicatorVertices)));
    }

    void InitVertexLayouts()
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
      glVertexArrayVertexBuffer(batchVAO, 0, vertexBuffer->GetAPIHandle(), 0, sizeof(Vertex));
      glVertexArrayElementBuffer(batchVAO, indexBuffer->GetAPIHandle());

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
      glVertexArrayVertexBuffer(axisVao, 0, axisVbo->GetAPIHandle(), 0, 6 * sizeof(float));
    }

    void InitTextures()
    {
      // these are some commonly used blue noise textures
      TextureManager::Get()->AddTexture("blueNoiseRGB",
        *LoadTexture2D("BlueNoise/16_16/LDR_RGB1_0.png", Format::R8G8B8A8_UNORM));
      tonemap.blueNoiseView = TextureView::Create(*TextureManager::Get()->GetTexture("blueNoiseRGB"), "BlueNoiseRGBView");
      TextureManager::Get()->AddTexture("blueNoiseR",
        *LoadTexture2D("BlueNoise/32_32/HDR_L_0.png", Format::R16_UNORM));
      TextureManager::Get()->AddTexture("blueNoiseBig",
        *LoadTexture2D("BlueNoise/1024_1024/LDR_RGBA_0.png", Format::R16G16B16A16_UNORM));
      blueNoiseRView = TextureView::Create(*TextureManager::Get()->GetTexture("blueNoiseR"));
      blueNoiseBigView = TextureView::Create(*TextureManager::Get()->GetTexture("blueNoiseBig"));
      SamplerState samplerState{};
      samplerState.asBitField.addressModeU = AddressMode::REPEAT;
      samplerState.asBitField.addressModeV = AddressMode::REPEAT;
      tonemap.blueNoiseSampler.emplace(*TextureSampler::Create(samplerState, "BlueNoiseRGBSampler"));

      // TODO: temp, inits the skybox cubemap
      const std::string_view faces[6] =
      {
        "autumn_sky_hdr/px.hdr",
        "autumn_sky_hdr/nx.hdr",
        "autumn_sky_hdr/py.hdr",
        "autumn_sky_hdr/ny.hdr",
        "autumn_sky_hdr/pz.hdr",
        "autumn_sky_hdr/nz.hdr",
      };
      env.skyboxMemory = LoadTextureCube(faces, 0, 0, true);
      env.skyboxMemory->GenMipmaps();
      env.skyboxView = TextureView::Create(*env.skyboxMemory, "skycube view");
      SamplerState cubesamplerState{};
      cubesamplerState.asBitField.addressModeU = AddressMode::MIRRORED_REPEAT;
      cubesamplerState.asBitField.addressModeV = AddressMode::MIRRORED_REPEAT;
      cubesamplerState.asBitField.addressModeW = AddressMode::MIRRORED_REPEAT;
      env.skyboxSampler = TextureSampler::Create(cubesamplerState, "skycube sampler");

      // init the reflection probe textures
      TextureCreateInfo cubeMemInfo
      {
        .imageType = ImageType::TEX_CUBEMAP,
        .format = probeData.colorFormat,
        .extent = { probeData.imageSize.width, probeData.imageSize.height, 1 },
        .mipLevels = 1,
        .arrayLayers = 6
      };
      //cubeMemInfo.mipLevels = glm::floor(glm::log2(glm::max((float)cubeMemInfo.extent.width, (float)cubeMemInfo.extent.height))) + 1;
      probeData.colorCube = Texture::Create(cubeMemInfo, "Cube Color");
      probeData.colorCubeView = TextureView::Create(*probeData.colorCube, "Cube Color View");

      cubeMemInfo.format = probeData.depthFormat;
      probeData.depthCube = Texture::Create(cubeMemInfo, "Cube Depth");

      cubeMemInfo.format = probeData.distanceFormat;
      probeData.distanceCube = Texture::Create(cubeMemInfo, "Cube Depth Distance");
      probeData.distanceCubeView = TextureView::Create(*probeData.distanceCube, "Cube Depth Distance View");

      const glm::vec3 faceDirs[6] =
      {
        { 1, 0, 0 },
        { -1, 0, 0 },
        { 0, 1, 0 },
        { 0, -1, 0 },
        { 0, 0, 1 },
        { 0, 0, -1 }
      };
      const Constants::CardinalNames upDirs[6] =
      {
        Constants::CardinalNames::NegY,
        Constants::CardinalNames::NegY,
        Constants::CardinalNames::PosZ,
        Constants::CardinalNames::NegZ,
        Constants::CardinalNames::NegY,
        Constants::CardinalNames::NegY,
      };
      for (uint32_t i = 0; i < 6; i++)
      {
        TextureViewCreateInfo cubeViewInfo
        {
          .viewType = ImageType::TEX_2D,
          .format = probeData.colorFormat,
          .minLevel = 0,
          .numLevels = 1,
          .minLayer = i,
          .numLayers = 1
        };
        probeData.colorViews[i].emplace(*TextureView::Create(cubeViewInfo, *probeData.colorCube));

        cubeViewInfo.format = probeData.depthFormat;
        probeData.depthViews[i].emplace(*TextureView::Create(cubeViewInfo, *probeData.depthCube));

        cubeViewInfo.format = probeData.distanceFormat;
        probeData.distanceViews[i].emplace(*TextureView::Create(cubeViewInfo, *probeData.distanceCube));

        probeData.cameras[i].proj = MakeInfReversedZProjRH(glm::radians(90.0f), 1.0f, 0.1f);
        probeData.cameras[i].viewInfo.SetForwardDir(faceDirs[i]);
        probeData.cameras[i].viewInfo.upDir = upDirs[i];
        RenderAttachment colorAttachment
        {
          .textureView = &*probeData.colorViews[i],
          .clearEachFrame = false,
        };
        RenderAttachment depthAttachment
        {
          .textureView = &*probeData.depthViews[i],
          .clearEachFrame = true,
          .clearValue = ClearValue {.depthStencil = {.depth = 0.0f } }
        };
        probeData.renderViews[i] = RenderView
        {
          .renderInfo = RenderInfo
          {
            .offset = { 0, 0 },
            .size = probeData.imageSize,
            .colorAttachments = { colorAttachment },
            .depthAttachment = depthAttachment
          },
          .camera = &probeData.cameras[i],
          .mask = RenderMaskBit::None
        };
        //scene->RegisterRenderView("ProbeFace" + std::to_string(i), testView);
      }
    }

    void DrawReflections()
    {
      DebugMarker markerRefl("Draw reflections");

      FX::ReflectionsCommonParameters commonParams
      {
        .gbDepth = *gBuffer.depthTexView,
        .gbColor = *gBuffer.colorTexView,
        .gbNormal = *gBuffer.normalTexView,
        .gbPBR = *gBuffer.PBRTexView,
        .scratchSampler = *reflect.scratchSampler,
        .scratchSampler2 = *reflect.scratchSampler2,
        .camera = gBuffer.camera
      };

      reflect.fbo->SetAttachment(Attachment::COLOR_0, *reflect.texView[0], 0);
      if (reflectionsModeCvar.Get() >= REFLECTION_MODE_PARALLAX_CUBE_THRESHOLD)
      {
        DebugMarker marker("Cube Traced Reflections");
        MEASURE_GPU_TIMER_STAT(ReflectionsTrace);

        FX::TraceCubemapReflectionsParameters traceParams
        {
          .common = commonParams,
          .target = *reflect.texView[0],
          .cameras = probeData.cameras,
          .probeColor = *probeData.colorCubeView,
          .probeDistance = *probeData.distanceCubeView,
          .depthViews = probeData.depthViews,
          .distanceViews = probeData.distanceViews,
          .skybox = *env.skyboxView,
          .blueNoise = *blueNoiseBigView
        };
        FX::TraceCubemapReflections(traceParams);
      }
      else
      {
        DebugMarker marker("Cube Sampled Reflections");
        MEASURE_GPU_TIMER_STAT(ReflectionsSample);

        FX::SampleCubemapReflectionsParameters sampleParams
        {
          .common = commonParams,
          .target = *reflect.texView[0],
          .env = reflectionsModeCvar.Get() >= REFLECTION_MODE_CUBE_THRESHOLD ? *probeData.colorCubeView : *env.skyboxView,
          .blueNoise = *blueNoiseBigView
        };
        FX::SampleCubemapReflections(sampleParams);
      }

      {
        DebugMarker marker("Denoise reflections");
        MEASURE_GPU_TIMER_STAT(ReflectionsDenoise);

        if (reflect.atrous.num_passes > 0 && reflectionsDenoiseEnableCvar.Get() != 0.0f)
        {
          FX::DenoiseReflectionsParameters denoiseParams
          {
            .common = commonParams,
            .target = *reflect.texView[0],
            .scratchTexture = *reflect.texView[1],
            .atrousParams
            {
              .passes = reflect.atrous.num_passes,
              .nPhi = reflect.atrous.n_phi,
              .pPhi = reflect.atrous.p_phi,
              .stepWidth = reflect.atrous.step_width,
              .kernel = reflect.atrous.kernel,
              .offsets = reflect.atrous.offsets
            }
          };
          FX::DenoiseReflections(denoiseParams);
        }
      }

      {
        DebugMarker marker("Composite reflections");
        MEASURE_GPU_TIMER_STAT(ReflectionsComposite);

        FX::CompositeReflectionsParameters compositeParams
        {
          .common = commonParams,
          .source = *reflect.texView[0],
          .target = *gBuffer.colorTexView
        };
        FX::CompositeReflections(compositeParams);
      }
    }

    // call when the window needs to be recreated, like when full screen mode is toggled
    GLFWwindow* CreateWindow(bool fullscreen)
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

    void SetFramebufferSize(uint32_t width, uint32_t height)
    {
      windowWidth = glm::max(width, 1u);
      windowHeight = glm::max(height, 1u);
      InitFramebuffers();
    }

    void SetRenderingScale(float scale)
    {
      renderScale = scale;
      renderWidth = glm::max(static_cast<uint32_t>(windowWidth * renderScale), 1u);
      renderHeight = glm::max(static_cast<uint32_t>(windowHeight * renderScale), 1u);
      InitFramebuffers();
    }

    void SetReflectionsRenderScale(float scale)
    {
      reflect.renderScale = scale;
      reflect.fboSize.width = glm::max(static_cast<uint32_t>(renderWidth * scale), 1u);
      reflect.fboSize.height = glm::max(static_cast<uint32_t>(renderHeight * scale), 1u);
      InitReflectionFramebuffer();
    }

    GLFWwindow* const* Init()
    {
      window_ = InitContext();

      InitVertexBuffers();
      InitFramebuffers();
      InitVertexLayouts();
      CompileShaders();
      InitTextures();

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
  }

  void SetFramebufferDrawBuffersAuto(Framebuffer& framebuffer, const RenderInfo& renderInfo, size_t maxCount)
  {
    Attachment drawBuffers[RenderInfo::maxColorAttachments]{ Attachment::NONE };
    for (size_t i = 0, j = 0; i < RenderInfo::maxColorAttachments; i++)
    {
      if (const auto& attachment = renderInfo.colorAttachments[i]; attachment.has_value())
      {
        framebuffer.SetAttachment(Attachment::COLOR_0 + i, *attachment->textureView, 0);
        drawBuffers[j] = Attachment::COLOR_0 + i;
        if (j++ > maxCount) break;
      }
    }
    framebuffer.SetDrawBuffers(drawBuffers);
  }
}