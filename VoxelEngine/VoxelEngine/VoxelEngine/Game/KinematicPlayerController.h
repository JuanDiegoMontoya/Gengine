#pragma once
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Camera.h>
#include <CoreEngine/Components/Physics.h>
#include <CoreEngine/Components/Transform.h>


class KinematicPlayerController : public ScriptableEntity
{
public:
  virtual void OnCreate() override
  {
  }

  virtual void OnDestroy() override
  {

  }

  virtual void OnUpdate(float dt) override
  {
    if (Input::IsKeyPressed(GLFW_KEY_GRAVE_ACCENT))
    {
      activeCursor = !activeCursor;
      Input::SetCursorVisible(activeCursor);
    }

    auto& cam = GetComponent<Component::Camera>();// *CameraSystem::ActiveCamera;
    const auto& transform = GetComponent<Component::Transform>();
    //cam.SetPos(transform.GetTranslation()); // TODO: TEMP BULLSHIT
    auto& controller = GetComponent<Component::CharacterController>();

    const glm::vec2 xzForward = glm::normalize(glm::vec2(cam.GetForward().x, cam.GetForward().z));
    const glm::vec2 xzRight = glm::normalize(glm::vec2(-xzForward.y, xzForward.x));

    float maxXZSpeed = normalSpeed;

    // speed modifiers
    float acceleration = flags & Physics::ControllerCollisionFlag::COLLISION_DOWN ? accelerationGround : accelerationAir;
    if (Input::GetInputAxis("Sprint") != 0.0f)
    {
      maxXZSpeed = fastSpeed;
    }
    if (Input::GetInputAxis("Crouch") != 0.0f)
    {
      if (acceleration == accelerationGround)
        acceleration = accelerationGroundSlow;
      maxXZSpeed = slowSpeed;
    }

    float currSpeed = acceleration * dt;
    bool speeding = false;
    glm::vec2 xzForce{ 0 };
    //if (Input::IsKeyDown(GLFW_KEY_W))
    //  xzForce += xzForward;
    //if (Input::IsKeyDown(GLFW_KEY_S))
    //  xzForce -= xzForward;
    //if (Input::IsKeyDown(GLFW_KEY_A))
    //  xzForce += xzRight;
    //if (Input::IsKeyDown(GLFW_KEY_D))
    //  xzForce -= xzRight;
    xzForce += xzForward * Input::GetInputAxis("Forward");
    xzForce += xzRight * Input::GetInputAxis("Strafe");
    if (xzForce != glm::vec2(0))
      xzForce = glm::normalize(xzForce) * currSpeed;
    if (Input::IsKeyDown(GLFW_KEY_T))
    {
      velocity += cam.GetForward() * dt * 100.f;
      speeding = true;
    }

    // limit XZ speed
    glm::vec2 tempXZvel{ velocity.x + xzForce[0], velocity.z + xzForce[1] };
    float curSpeed = glm::length(glm::vec2(velocity.x, velocity.z));
    //velocity.x += xzForce[0];
    //velocity.z += xzForce[1];
    if (auto len = glm::length(tempXZvel); (len > curSpeed && len > maxXZSpeed) && !speeding)
    {
      tempXZvel = tempXZvel / len * curSpeed;
    }
    velocity.x = tempXZvel.x;
    velocity.z = tempXZvel.y;

    glm::vec3 startPosition = controller.Interface().GetPosition();
    glm::vec3 expectedPosition = startPosition;

    accumulator += dt;
    dt = tick;
    while (accumulator > tick)
    {
      accumulator -= tick;

      velocity.y += gravity * dt;
      glm::vec2 velXZ{ velocity.x, velocity.z };
      float deceleration = 0;
      expectedPosition += velocity * dt;
      flags = controller.Interface().Move(velocity * dt, dt);
      if (flags & Physics::ControllerCollisionFlag::COLLISION_DOWN || flags & Physics::ControllerCollisionFlag::COLLISION_UP)
      {
        velocity.y = 0;
        // jump if colliding below
        if (flags & Physics::ControllerCollisionFlag::COLLISION_DOWN && Input::IsKeyDown(GLFW_KEY_SPACE))
          velocity.y = jumpVel;

        deceleration = decelerationGround;
      }
      else
      {
        // use air friction
        deceleration = decelerationAir;
      }

      // use friction if no movement was inputted, or if above max speed
      if (deceleration != 0 && (xzForce == glm::vec2(0) || glm::length(velXZ) > maxXZSpeed))
      {
        //glm::vec2 dV = velXZ * glm::clamp((1.0f - decceleration) * dt, 0.001f, 1.0f); // exponential friction
        glm::vec2 dV;
        if (glm::all(glm::epsilonEqual(velXZ, glm::vec2(0), .001f)))
          dV = { 0,0 };
        else
          dV = glm::clamp(glm::abs(glm::normalize(velXZ)) * deceleration * dt, 0.001f, 1.0f);// linear friction
        velXZ -= glm::min(glm::abs(dV), glm::abs(velXZ)) * glm::sign(velXZ);
      }
      velocity.x = velXZ.x;
      velocity.z = velXZ.y;

      if (flags & Physics::ControllerCollisionFlag::COLLISION_SIDES)
      {
      }

      // if the actual position is less than if you added velocity to previous position (i.e. you collided with something),
      // then lower the velocity correspondingly
      glm::vec3 actualPosition = controller.Interface().GetPosition();
      glm::vec3 actualVelocity = (actualPosition - startPosition) / dt;
      if (glm::length(glm::vec2(actualVelocity.x, actualVelocity.z)) < glm::length(glm::vec2(velocity.x, velocity.z)))
      {
        velocity.x = actualVelocity.x;
        velocity.z = actualVelocity.z;
      }
      //printf("%f, %f, %f\n", velocity.x, velocity.y, velocity.z);
    }

    // mouse look controls
    if (!activeCursor)
    {
      auto pyr = cam.GetEuler(); // Pitch, Yaw, Roll
      cam.SetYaw(pyr.y + Input::GetScreenOffset().x);
      cam.SetPitch(glm::clamp(pyr.x + Input::GetScreenOffset().y, -89.0f, 89.0f));
    }
    //pyr = CameraSystem::GetEuler(); // oof

    //glm::vec3 temp;
    //temp.x = cos(glm::radians(pyr.x)) * cos(glm::radians(pyr.y));
    //temp.y = sin(glm::radians(pyr.x));
    //temp.z = cos(glm::radians(pyr.x)) * sin(glm::radians(pyr.y));
    //CameraSystem::SetFront(glm::normalize(temp));
    //CameraSystem::SetDir(CameraSystem::GetFront());
  }

  // instantly sets Y velocity to this when jumping
  const float jumpVel = 8.0f;

  // amount of velocity to lose per second from gravity
  const float gravity = -15.f;

  // amount of velocity to gain/lose per second when moving/not moving
  const float accelerationGround = 60.0f;
  const float accelerationGroundSlow = 30.0f;
  const float accelerationAir = 20.0f;
  const float decelerationGround = 60.0f;
  const float decelerationAir = 3.0f;

  // max speed on the XZ plane when moving
  const float slowSpeed = 2.0f;
  const float normalSpeed = 4.0f;
  const float fastSpeed = 6.0f;

  // displacement after one second
  glm::vec3 velocity{ 0, 0, 0 };

  // fix step bookkeeping
  const float tick = 1.0f / 200.0f;
  float accumulator = 0;
  Physics::ControllerCollisionFlags flags{};

  // misc
  bool activeCursor{ false };
};