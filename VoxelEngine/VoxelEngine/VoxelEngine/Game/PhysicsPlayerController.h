#pragma once
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Components.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Camera.h>

class PhysicsPlayerController : public ScriptableEntity
{
public:
  virtual void OnCreate() override
  {
    auto& physics = GetComponent<Components::Physics>();
    physics.Interface().SetMaxVelocity(maxSpeed);
    physics.Interface().SetLockFlags(Physics::LockFlag::LOCK_ANGULAR_X | Physics::LockFlag::LOCK_ANGULAR_Y | Physics::LockFlag::LOCK_ANGULAR_Z);
  }

  virtual void OnDestroy() override
  {

  }

  virtual void OnUpdate(float dt) override
  {
    auto& cam = *GetComponent<Components::Camera>().cam;
    const auto& transform = GetComponent<Components::Transform>();
    cam.SetPos(transform.GetTranslation()); // TODO: TEMP BULLSHIT

    auto& physics = GetComponent<Components::Physics>();

    glm::vec2 xzForce{0};// { physics.velocity.x, physics.velocity.z };
    const glm::vec2 xzForward = glm::normalize(glm::vec2(cam.GetDir().x, cam.GetDir().z));
    const glm::vec2 xzRight = glm::normalize(glm::vec2(xzForward.y, -xzForward.x));

    float currSpeed = speed * dt;
    xzForce *= glm::pow(.90f, dt * 100);

    if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
      currSpeed *= 10;
    if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL))
      currSpeed /= 10;
    if (Input::IsKeyDown(GLFW_KEY_W))
      xzForce += xzForward * currSpeed;
    if (Input::IsKeyDown(GLFW_KEY_S))
      xzForce -= xzForward * currSpeed;
    if (Input::IsKeyDown(GLFW_KEY_A))
      xzForce += xzRight * currSpeed;
    if (Input::IsKeyDown(GLFW_KEY_D))
      xzForce -= xzRight * currSpeed;

    if (Input::IsKeyPressed(GLFW_KEY_SPACE))
      physics.Interface().AddForce({ 0, jumpForce, 0 }); // TODO: should be +=, but can only trigger when player is on ground
    physics.Interface().AddForce({ xzForce.x, 0, xzForce.y });
    //if (auto len = glm::length(xzVel); len > maxSpeed)
    //  xzVel = (xzVel / len) * maxSpeed;
    //physics.velocity.x = xzVel[0];
    //physics.velocity.z = xzVel[1];

    auto pyr = cam.GetEuler(); // Pitch, Yaw, Roll
    cam.SetYaw(pyr.y + Input::GetScreenOffset().x);
    cam.SetPitch(glm::clamp(pyr.x + Input::GetScreenOffset().y, -89.0f, 89.0f));
    pyr = cam.GetEuler(); // oof

    glm::vec3 temp;
    temp.x = cos(glm::radians(pyr.x)) * cos(glm::radians(pyr.y));
    temp.y = sin(glm::radians(pyr.x));
    temp.z = cos(glm::radians(pyr.x)) * sin(glm::radians(pyr.y));
    cam.SetFront(glm::normalize(temp));
    cam.SetDir(cam.GetFront());
  }

  float jumpForce = 30000.0f;
  float speed = 60000.f;
  float maxSpeed = 5.f;
};