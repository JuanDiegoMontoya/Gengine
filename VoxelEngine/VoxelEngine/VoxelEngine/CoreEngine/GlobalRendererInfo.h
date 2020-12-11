#pragma once
#include <memory>
#include <Graphics/GraphicsIncludes.h>

class UBO;
class Camera;

struct GlobalRendererInfo
{
  GlobalRendererInfo() = default;

  void SetCamera(Camera* camera);

  struct GLFWwindow* window = nullptr;
  Camera* activeCamera = nullptr;
  std::unique_ptr<UBO> ubo = nullptr;

  struct UBOData
  {
    glm::mat4 viewProj{ 1 };
    glm::mat4 invView{ 1 };
    glm::mat4 invProj{ 1 };
    bool debug = true;
  };
};