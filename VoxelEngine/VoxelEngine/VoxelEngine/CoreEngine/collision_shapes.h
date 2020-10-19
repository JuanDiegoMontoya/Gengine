#pragma once
#include "Camera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>
#undef GLM_ENABLE_EXPERIMENTAL
//#include <numeric>

// abstract class
//struct Collider
//{
//  template<typename T>
//  bool CheckCollision(const T& other);
//};

// AABB
#pragma optimize("", off)
struct Box
{
  Box() {};

  // 1x1x1 block
  Box(const glm::vec3& wpos)
    : min(), max(), blockpos()
  {
    min = wpos - .5f;
    max = wpos + .5f;
    min += .5f;
    max += .5f;
    blockpos = wpos;
  }


  // .5 x .5 x .5 camera
  Box(const Camera& c)
    : min(), max(), blockpos(0)
  {
    const auto& p = c.GetPos();
    min = p - .25f;
    max = p + .25f;
    //max.y *= 2;
    min.y -= 1.25f;
  }


  bool IsColliding(const Box& b) const
  {
    return  (this->min.x <= b.max.x && this->max.x >= b.min.x) &&
            (this->min.y <= b.max.y && this->max.y >= b.min.y) &&
            (this->min.z <= b.max.z && this->max.z >= b.min.z);
  }


  glm::vec3 GetPosition() const
  {
    return (max + min) / 2.f;
  }


  glm::vec3 GetScale() const
  {
    return (max - min) / 2.f;
  }


  glm::vec3 min{ 0, 0, 0 };
  glm::vec3 max{ 0, 0, 0 };
  glm::ivec3 blockpos{ 0, 0, 0 };
};


//https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/
// TODO: SIMD (use glm::vec functions instead of repetitive if/else)
struct SweptAABB
{
  SweptAABB() {}

  // collision test and response, returns true if there was a collision
  bool CheckCollisionSweptAABBDeflect(const Box& b2)
  {
    auto [collisionTime, normal] = TestSweptAABB(b2);
    if (collisionTime == 1)
      return false;
    this->min -= this->vel * collisionTime;
    this->max -= this->vel * collisionTime;
    float remainingTime = 1.0f - collisionTime;
    
    // reduce velocity by amt of elapsed time, then reflect velocity over collision normal
    this->vel *= remainingTime;
    if (glm::abs(normal.x) > 0.0001f)
      this->vel.x *= -1;
    if (glm::abs(normal.y) > 0.0001f)
      this->vel.y *= -1;
    if (glm::abs(normal.z) > 0.0001f)
      this->vel.z *= -1;

    return true;
  }

  // collision test and response, returns true if there was a collision
  bool CheckCollisionSweptAABBPush(const Box& b2)
  {
    auto [collisionTime, normal] = TestSweptAABB(b2);
    if (collisionTime == 1)
      return false;
    this->min += this->vel * collisionTime;
    this->max += this->vel * collisionTime;
    float remainingTime = 1.0f - collisionTime;

    float magnitude = glm::length(this->vel) * remainingTime;
    this->vel = glm::normalize(this->vel);

    // TODO: swap systems of normal (how?)
    float dotprod = glm::dot(this->vel, normal);

    this->vel = dotprod * normal * magnitude;

    return true;
  }

  // collision test and response, returns true if there was a collision
  bool CheckCollisionSweptAABBSlide(const Box& b2)
  {
    auto [collisionTime, normal] = TestSweptAABB(b2);
    if (collisionTime == 1)
      return false;
    this->min += this->vel * collisionTime;
    this->max += this->vel * collisionTime;
    float remainingTime = 1.0f - collisionTime;

    // TODO: swap systems of normal (how?)
    float dotprod = glm::dot(this->vel, normal) * remainingTime;
    this->vel = dotprod * normal;

    return true;
  }

  Box GetSweptAABB()
  {
    Box aabb;
    aabb.min = glm::min(min, min + vel);
    aabb.max = glm::max(max, max + vel);
    return aabb;
  }

  // generic test
  std::pair<float, glm::vec3> TestSweptAABB(const Box& b2)
  {
    glm::vec3 invEntry;
    glm::vec3 invExit;
    glm::vec3 normal;
    const auto& b1 = *this;

    if (b1.vel.x > 0)
    {
      invEntry.x = b2.min.x - b1.max.x;
      invExit.x = b2.max.x - b1.min.x;
    }
    else
    {
      invEntry.x = b2.max.x - b1.min.x;
      invExit.x = b2.min.x - b1.max.x;
    }

    if (b1.vel.y > 0)
    {
      invEntry.y = b2.min.y - b1.max.y;
      invExit.y = b2.max.y - b1.min.y;
    }
    else
    {
      invEntry.y = b2.max.y - b1.min.y;
      invExit.y = b2.min.y - b1.max.y;
    }

    if (b1.vel.z > 0)
    {
      invEntry.z = b2.min.z - b1.max.z;
      invExit.z = b2.max.z - b1.min.z;
    }
    else
    {
      invEntry.z = b2.max.z - b1.min.z;
      invExit.z = b2.min.z - b1.max.z;
    }

    glm::vec3 entry;
    glm::vec3 exit;

    if (b1.vel.x == 0)
    {
      entry.x = -std::numeric_limits<float>::infinity();
      exit.x = std::numeric_limits<float>::infinity();
    }
    else
    {
      entry.x = invEntry.x / b1.vel.x;
      exit.x = invExit.x / b1.vel.x;
    }

    if (b1.vel.y == 0)
    {
      entry.y = -std::numeric_limits<float>::infinity();
      exit.y = std::numeric_limits<float>::infinity();
    }
    else
    {
      entry.y = invEntry.y / b1.vel.y;
      exit.y = invExit.y / b1.vel.y;
    }

    if (b1.vel.z == 0)
    {
      entry.z = -std::numeric_limits<float>::infinity();
      exit.z = std::numeric_limits<float>::infinity();
    }
    else
    {
      entry.z = invEntry.z / b1.vel.z;
      exit.z = invExit.z / b1.vel.z;
    }

    float entryTime = glm::compMax(entry);
    float exitTime = glm::compMin(exit);

    // no collision
    //if (entryTime > exitTime) return 1.0f; // This check was correct.
    //if (entryX < 0.0f && entryY < 0.0f) return 1.0f;
    //if (entryX < 0.0f) {
    //  // Check that the bounding box started overlapped or not.
    //  if (s.max.x < t.min.x || s.min.x > t.max.x) return 1.0f;
    //}
    //if (entryY < 0.0f) {
    //  // Check that the bounding box started overlapped or not.
    //  if (s.max.y < t.min.y || s.min.y > t.max.y) return 1.0f;
    //}
    using namespace glm;
    if (entryTime > exitTime ||
      all(lessThan(entry, vec3(0))) ||
      any(greaterThan(entry, vec3(1))))
    {
      normal = vec3(0);
      return { 1.0f, normal };
    }
    else // there was a collision
    {
      // calculate normal of collision
      if (entry.x > entry.y && entry.x > entry.z)
      {
        if (invEntry.x < 0)
          normal = vec3(1, 0, 0);
        else
          normal = vec3(-1, 0, 0);
      }
      else if (entry.y > entry.x && entry.y > entry.z)
      {
        if (invEntry.y < 0)
          normal = vec3(0, 1, 0);
        else
          normal = vec3(0, -1, 0);
      }
      else
      {
        if (invEntry.z < 0)
          normal = vec3(0, 0, 1);
        else
          normal = vec3(0, 0, -1);
      }

      return { entryTime, normal };
    }
  }


  glm::vec3 GetPosition() const
  {
    return (max + min) / 2.f;
  }
  glm::vec3 GetScale() const
  {
    return (max - min) / 2.f;
  }

  glm::vec3 vel{ 0 };
  glm::vec3 min{ 0 };
  glm::vec3 max{ 0 };
};
#pragma optimize("", on)