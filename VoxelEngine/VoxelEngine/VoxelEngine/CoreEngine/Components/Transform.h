#pragma once
#include "../GAssert.h"
#include "../Entity.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Component
{
  struct Transform
  {
  public:
    const auto& GetTranslation() const { return translation; }
    const auto& GetRotation() const { return rotation; }
    const auto& GetScale() const { return scale; }
    auto GetModel() const
    {
      /*return model;*/
      return glm::translate(glm::mat4(1), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1), scale);
    }

    auto IsDirty() const { return isDirty; }
    void SetTranslation(const glm::vec3& vec) { translation = vec; isDirty = true; }
    void SetRotation(const glm::mat4& mat) { rotation = glm::quat_cast(mat); isDirty = true; }
    void SetRotation(const glm::quat& q) { rotation = q; isDirty = true; }
    void SetScale(const glm::vec3& vec) { scale = vec; isDirty = true; }
    void SetModel() { isDirty = false; }

  private:
    glm::vec3	translation{ 0 };
    glm::quat rotation{ 1, 0, 0, 0 };
    glm::vec3	scale{ 1, 1, 1 };

    glm::vec3 forward{ 0, 0, 1 };
    glm::vec3 up{ 0, 0, 1 };
    glm::vec3 right{ 0, 0, 1 };

    bool isDirty = false;
  };

  struct Parent
  {
    Entity entity{};
  };

  struct Children
  {
    void AddChild(Entity child)
    {
      ASSERT_MSG(std::count(children.begin(), children.end(), child) == 0, "That entity is already a child of this!");
      children.push_back(child);
    }

    void RemoveChild(Entity child)
    {
      ASSERT_MSG(std::count(children.begin(), children.end(), child) == 1, "That entity is not a child of this!");
      std::erase(children, child);
    }

    const std::vector<Entity> GetChildren() const { return children; }

    size_t size() const { return children.size(); }

    unsigned cachedHeight{};

  private:
    friend class Entity;
    friend class Scene;
    std::vector<Entity> children{};
  };

  // if entity has parent, controls transform w.r.t. the parent's
  struct LocalTransform
  {
    Transform transform;
  };
}