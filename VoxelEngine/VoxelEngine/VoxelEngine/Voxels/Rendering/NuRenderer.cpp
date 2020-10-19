#include <Rendering/NuRenderer.h>
//#include "World.h"
#include <Engine.h>
#include <Graphics/shader.h>
#include <Camera.h>
#include <Input.h>
#include <Graphics/dib.h>
#include <Rendering/ChunkRenderer.h>
#include <block.h>
#include <Rendering/TextureArray.h>
#include <Texture2D.h>
#include <Refactor/sun.h>

#include <GraphicsSystem.h>

namespace NuRenderer
{
  namespace
  {
    // block textures
    std::unique_ptr<TextureArray> textures;
    std::unique_ptr<Texture2D> blueNoise64;
  }


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


  TextureArray* GetBlockTextures()
  {
    return textures.get();
  }


  void Init()
  {
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
    glfwSwapInterval(0);

    std::vector<std::string> texs;
    for (const auto& prop : Block::PropertiesTable)
    {
      texs.push_back(std::string(prop.name) + ".png");
    }
    textures = std::make_unique<TextureArray>(std::span(texs.data(), texs.size()), glm::ivec2(32));

    //blueNoise64 = std::make_unique<Texture2D>("BlueNoise/64_64/LDR_LLL1_0.png");
    blueNoise64 = std::make_unique<Texture2D>("BlueNoise/256_256/LDR_LLL1_0.png");

    //GLint count;
    //glGetIntegerv(GL_NUM_EXTENSIONS, &count);
    //for (GLint i = 0; i < count; ++i)
    //{
    //  const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
    //  if (strcmp(extension, "GL_NVX_gpu_memory_info") == 0)
    //    nvUsageEnabled = true;
    //}

    CompileShaders();
    //Engine::PushRenderCallback(DrawAll, RenderOrder::RenderDrawAll);
    //Engine::PushRenderCallback(ChunkRenderer::Update, RenderOrder::RenderChunkRenderUpdate);
    //Engine::PushRenderCallback(Clear, RenderOrder::RenderClear);
    //Engine::PushRenderCallback(Renderer::drawSky, RenderOrder::RenderSky);

    activeSun_ = std::make_unique<Sun>();
  }


  //void RecompileShaders()
  //{
  //  for (auto& shader : Shader::shaders)
  //  {
  //    std::string vs = shader.second->vsPath.c_str();
  //    std::string fs = shader.second->fsPath.c_str();
  //    delete shader.second;
  //    shader.second = new Shader(vs.c_str(), fs.c_str());
  //  }
  //}

  void CompileShaders()
  {
    Shader::shaders["chunk_optimized"].emplace(Shader(
      {
        { "chunk_optimized.vs", GL_VERTEX_SHADER },
        { "chunk_optimized.fs", GL_FRAGMENT_SHADER }
      }));
    //Shader::shaders["chunk_splat"] = new Shader("chunk_splat.vs", "chunk_splat.fs");
    Shader::shaders["compact_batch"].emplace(Shader(
      { { "compact_batch.cs", GL_COMPUTE_SHADER } }));
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

    Shader::shaders["sun"].emplace(Shader("flat_sun.vs", "flat_sun.fs"));
    Shader::shaders["axis"].emplace(Shader("axis.vs", "axis.fs"));
    Shader::shaders["flat_color"].emplace(Shader("flat_color.vs", "flat_color.fs"));
    //Shader::shaders["chunk_render_cull"] = new Shader("chunk_render_cull.vs", "chunk_render_cull.fs");
  }


  void Clear()
  {
    drawCalls = 0;
    auto cc = glm::vec3(.529f, .808f, .922f);
    glClearColor(cc.r, cc.g, cc.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }


  void DrawAll()
  {
    //PERF_BENCHMARK_START;
    //Clear();

    if (settings.gammaCorrection)
      glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction

    if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
    {
      if (Input::IsKeyDown(GLFW_KEY_1))
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      if (Input::IsKeyDown(GLFW_KEY_2))
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      if (Input::IsKeyDown(GLFW_KEY_3))
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    {
      auto& sett = ChunkRenderer::settings;
      if (Input::IsKeyPressed(GLFW_KEY_5))
      {
        sett.freezeCulling = !sett.freezeCulling;
        std::cout << "Freeze culling " << std::boolalpha << sett.freezeCulling << '\n';
      }
      if (Input::IsKeyPressed(GLFW_KEY_6))
      {
        // TODO: shading/wireframe on this drawing mode
        sett.debug_drawOcclusionCulling = !sett.debug_drawOcclusionCulling;
        std::cout << "DbgDrawOccCulling " << std::boolalpha << sett.debug_drawOcclusionCulling << '\n';
      }
    }

    drawChunks();
    //splatChunks();
    drawChunksWater();
    NuRenderer::drawAxisIndicators();
    ChunkRenderer::DrawBuffers();
    //Renderer::postProcess();

    glDisable(GL_FRAMEBUFFER_SRGB);
    ChunkRenderer::Update();

    //PERF_BENCHMARK_END;
  }


  void drawChunks()
  {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // don't forget to reset original culling face

    // render blocks in each active chunk
    auto& currShader = Shader::shaders["chunk_optimized"];
    currShader->Use();

    Camera* cam = Camera::ActiveCamera;
    float angle = glm::max(glm::dot(-glm::normalize(NuRenderer::activeSun_->GetDir()), glm::vec3(0, 1, 0)), 0.f);
    currShader->setFloat("sunAngle", angle);
    //printf("Angle: %f\n", angle);
    // currShader->setInt("textureAtlas", ...);

    // undo gamma correction for sky color
    static glm::vec3 skyColor(
      glm::pow(.529f, 2.2f),
      glm::pow(.808f, 2.2f),
      glm::pow(.922f, 2.2f));
    currShader->setVec3("viewPos", cam->GetPos());
    currShader->setFloat("fogStart", settings.fogStart);
    currShader->setFloat("fogEnd", settings.fogEnd);
    currShader->setVec3("fogColor", skyColor);
    currShader->setMat4("u_viewProj", cam->GetProj() * cam->GetView());

    textures->Bind(0);
    currShader->setInt("textures", 0);
    blueNoise64->Bind(1);
    currShader->setInt("blueNoise", 1);

    

    currShader->Use();
    //ChunkRenderer::RenderNorm();
    ChunkRenderer::Render();
    ChunkRenderer::GenerateDIB();
    ChunkRenderer::RenderOcclusion();
    drawCalls++;
  }

  void drawChunksWater()
  {

  }

  void drawAxisIndicators()
  {
    static VAO* axisVAO;
    static VBO* axisVBO;
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

      axisVAO = new VAO();
      axisVBO = new VBO(indicatorVertices, sizeof(indicatorVertices), GL_STATIC_DRAW);
      VBOlayout layout;
      layout.Push<float>(3);
      layout.Push<float>(3);
      axisVAO->AddBuffer(*axisVBO, layout);
    }
    /* Renders the axis indicator (a screen-space object) as though it were
      one that exists in the world for simplicity. */
    auto& currShader = Shader::shaders["axis"];
    currShader->Use();
    Camera* cam = Camera::ActiveCamera;
    currShader->setMat4("u_model", glm::translate(glm::mat4(1), cam->GetPos() + cam->GetFront() * 10.f)); // add scaling factor (larger # = smaller visual)
    currShader->setMat4("u_view", cam->GetView());
    currShader->setMat4("u_proj", cam->GetProj());
    glClear(GL_DEPTH_BUFFER_BIT); // allows indicator to always be rendered
    axisVAO->Bind();
    glLineWidth(2.f);
    glDrawArrays(GL_LINES, 0, 6);
    axisVAO->Unbind();
  }


  // draws a single quad over the entire viewport
  void drawQuad()
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

  void DrawCube()
  {
    static VAO* blockHoverVao = nullptr;
    static VBO* blockHoverVbo = nullptr;
    if (blockHoverVao == nullptr)
    {
      blockHoverVao = new VAO();
      blockHoverVbo = new VBO(Vertices::cube_norm_tex, sizeof(Vertices::cube_norm_tex));
      VBOlayout layout;
      layout.Push<float>(3);
      layout.Push<float>(3);
      layout.Push<float>(2);
      blockHoverVao->AddBuffer(*blockHoverVbo, layout);
    }
    //glClear(GL_DEPTH_BUFFER_BIT);
    //glDisable(GL_CULL_FACE);
    blockHoverVao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glEnable(GL_CULL_FACE);
  }
}