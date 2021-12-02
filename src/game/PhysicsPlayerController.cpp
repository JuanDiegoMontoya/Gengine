#include "gPCH.h"
#include "PhysicsPlayerController.h"
#include <engine/Input.h>
#include <engine/ecs/component/Physics.h>
#include <engine/ecs/component/Transform.h>
#include <engine/gfx/RenderView.h>
#include <engine/gfx/Camera.h>

void PhysicsPlayerController::OnCreate()
{
  auto& physics = GetComponent<Component::DynamicPhysics>();
  //physics.Interface().SetMaxVelocity(maxSpeed);
  physics.Interface().SetLockFlags(Physics::LockFlag::LOCK_ANGULAR_X | Physics::LockFlag::LOCK_ANGULAR_Y | Physics::LockFlag::LOCK_ANGULAR_Z);
  physics.Interface().SetMass(5);
}

void PhysicsPlayerController::OnUpdate(Timestep timestep)
{
  const auto& transform = GetComponent<Component::Transform>();
  auto* camera = GetScene()->GetRenderView("main")->camera;
  auto& vi = camera->viewInfo;
  const auto cameraDir = vi.GetForwardDir();
  vi.position = transform.GetTranslation();
  auto& physics = GetComponent<Component::DynamicPhysics>();

  glm::vec2 xzForce{ 0 };
  const glm::vec2 xzForward = glm::normalize(glm::vec2(cameraDir.x, cameraDir.z));
  const glm::vec2 xzRight = glm::normalize(glm::vec2(xzForward.y, -xzForward.x));
  //
  float currSpeed = normalForce * timestep.dt_effective;
  float maxSpeed = normalSpeed;
  if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
  {
    maxSpeed = fastSpeed;
    currSpeed = fastForce * timestep.dt_effective;
  }
  if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL))
  {
    maxSpeed = slowSpeed;
    currSpeed = slowForce * timestep.dt_effective;
  }
  if (Input::IsKeyDown(GLFW_KEY_T))
  {
    maxSpeed = 1000;
    physics.Interface().AddForce(cameraDir * 50.f, Physics::ForceMode::FORCE);
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

  vi.yaw += Input::GetScreenOffset().x;
  vi.pitch = glm::clamp(vi.pitch + Input::GetScreenOffset().y, glm::radians(-89.0f), glm::radians(89.0f));
}
