#pragma once
#include "Transform.h"
#include "../MeshUtils.h"
#include "../Entity.h"

#include <CoreEngine/TextureCube.h>
#include <memory>

namespace Components
{
  struct Camera
  {
    Camera(Entity);

    void AddCullingFlag(RenderFlags renderFlag)
    {
      cullingMask |= (uint64_t)renderFlag;
    }

    void RemoveCullingFlag(RenderFlags renderFlag)
    {
      cullingMask &= ~(uint64_t)renderFlag;
    }

    void SetPos(glm::vec3 pos) { translation = pos; }

    void SetYaw(float f) { yaw_ = f; }
    void SetPitch(float f) { pitch_ = f; }
    void SetDir(glm::vec3 d) { dir = d; }

    glm::vec3 GetEuler() { return { pitch_, yaw_, roll_ }; }

    glm::vec3 GetWorldPos()
    {
      Transform& tr = entity.GetComponent<Components::Transform>();
      return tr.GetTranslation() + translation;
    }

    const auto& GetLocalPos() { return translation; }

    auto GetForward()
    {
      dir.x = cos(glm::radians(pitch_)) * cos(glm::radians(yaw_));
      dir.y = sin(glm::radians(pitch_));
      dir.z = cos(glm::radians(pitch_)) * sin(glm::radians(yaw_));
      return dir;
    }

    glm::vec3	dir{ 0 };
    glm::vec3	translation{ 0 };

    float pitch_ = 16;
    float yaw_ = 255;
    float roll_ = 0;

    Entity entity{};

    bool frustumCulling = false;

    uint64_t cullingMask = (uint64_t)RenderFlags::NoRender;

    float fov = 70.0f;

    float zNear = 0.1f;
    float zFar = 1000.0f;

    std::unique_ptr<GFX::TextureCube> skybox{};
    GLuint renderTexture = 0;
  };

}