#pragma once
#include "MathIncludes.h"
#include <memory>

class Frustum;

class Camera
{
public:
  Camera();
  ~Camera();

  void Update(float dt);

  void UpdateViewMat();
  const glm::mat4& GetView() const { return view_; }
  const glm::mat4& GetProj() const { return proj_; }
  glm::mat4 GetProjView() { return proj_ * view_; }
  const glm::vec3& GetPos() const { return worldpos_; }
  const glm::vec3& GetDir() const { return dir_; }
  const Frustum* GetFrustum() const { return frustum_.get(); }
  float GetFov() const { return fovDeg_; }
  float GetNear() const { return near_; }
  float GetFar() const { return far_; }
  glm::vec3 GetEuler() const { return { pitch_, yaw_, roll_ }; }
  auto GetFront() const { return front; }
  auto GetUp() const { return up; }

  void SetPos(const glm::vec3& v) { worldpos_ = v; dirty_ = true; }
  void SetFar(float f) { far_ = f; GenProjection(); dirty_ = true; }
  void SetFront(const glm::vec3& f) { front = f; dirty_ = true; }
  void SetDir(const glm::vec3& v) { dir_ = v; dirty_ = true; }
  void SetYaw(float f) { yaw_ = f; dirty_ = true; }
  void SetPitch(float f) { pitch_ = f; dirty_ = true; }
  void GenProjection(float fovDeg = 80.f);

  static inline Camera* ActiveCamera = nullptr;

private:

  std::unique_ptr<Frustum> frustum_;
  glm::vec3 worldpos_ = glm::vec3(150, 50, 100);
  glm::vec3 dir_ = glm::vec3(-.22f, .22f, -.95f);
  glm::mat4 view_ = glm::mat4(1);
  glm::mat4 proj_;

  glm::vec3 up = glm::vec3(0, 1.f, 0);
  glm::vec3 front = glm::vec3(0, 0, -1.f);


  float speed_ = 3.5f;

  // view matrix info
  float pitch_ = 16;
  float yaw_ = 255;
  float roll_ = 0;

  // projection matrix info
  float fovDeg_ = 80.f;
  float near_ = .1f;
  float far_ = 300.f;

  bool dirty_ = true;
};