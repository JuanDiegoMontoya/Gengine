#pragma once
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Components.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Camera.h>

#pragma optimize("", off)

class PhysicsPlayerController : public ScriptableEntity
{
public:
  virtual void OnCreate() override
  {
    auto& physics = GetComponent<Components::DynamicPhysics>();
    //physics.Interface().SetMaxVelocity(maxSpeed);
    physics.Interface().SetLockFlags(Physics::LockFlag::LOCK_ANGULAR_X | Physics::LockFlag::LOCK_ANGULAR_Y | Physics::LockFlag::LOCK_ANGULAR_Z);
    physics.Interface().SetMass(5);
  }

  virtual void OnDestroy() override
  {

  }

  virtual void OnUpdate(float dt) override
  {
    auto& cam = *GetComponent<Components::Camera>().cam;
    const auto& transform = GetComponent<Components::Transform>();
    cam.SetPos(transform.GetTranslation()); // TODO: TEMP BULLSHIT
    auto& physics = GetComponent<Components::DynamicPhysics>();

    glm::vec2 xzForce{0};
    const glm::vec2 xzForward = glm::normalize(glm::vec2(cam.GetDir().x, cam.GetDir().z));
    const glm::vec2 xzRight = glm::normalize(glm::vec2(xzForward.y, -xzForward.x));
    //
    float currSpeed = normalForce * dt;
    float maxSpeed = normalSpeed;
    if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
    {
      maxSpeed = fastSpeed;
      currSpeed = fastForce * dt;
    }
    if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL))
    {
      maxSpeed = slowSpeed;
      currSpeed = slowForce * dt;
    }
    if (Input::IsKeyDown(GLFW_KEY_T))
    {
      maxSpeed = 1000;
      physics.Interface().AddForce(cam.GetDir() * 50.f, Physics::ForceMode::Force);
    }
    if (Input::IsKeyDown(GLFW_KEY_W))
      xzForce += xzForward * currSpeed;
    if (Input::IsKeyDown(GLFW_KEY_S))
      xzForce -= xzForward * currSpeed;
    if (Input::IsKeyDown(GLFW_KEY_A))
      xzForce += xzRight * currSpeed;
    if (Input::IsKeyDown(GLFW_KEY_D))
      xzForce -= xzRight * currSpeed;

    physics.Interface().AddForce({ xzForce.x, 0, xzForce.y });
    auto vel = physics.Interface().GetVelocity();
    if (Input::IsKeyPressed(GLFW_KEY_SPACE))
      physics.Interface().SetVelocity({ vel.x, jumpVel, vel.z });
    vel = physics.Interface().GetVelocity();
    glm::vec2 xzVel{ vel.x, vel.z };
    if (auto len = glm::length(xzVel); len > maxSpeed)
      xzVel = (xzVel / len) * maxSpeed;
    vel.x = xzVel[0];
    vel.z = xzVel[1];
    physics.Interface().SetVelocity(vel);

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

  const float jumpVel = 8.0f;

  const float slowSpeed = 2.f;
  const float fastSpeed = 10.f;
  const float normalSpeed = 5.f;
  
  const float slowForce = 60.f;
  const float fastForce = 400.f;
  const float normalForce = 200.f;
};