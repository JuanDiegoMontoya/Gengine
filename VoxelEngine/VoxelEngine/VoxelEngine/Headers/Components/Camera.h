#pragma once

#include <iostream>
#include "../Containers/Properties.h"
#include "Component.h"

typedef class UpdateEvent UpdateEvent;
class Frustum;

class Camera : public Component
{
public:
  static const ID componentType = cCamera;

  //PROPERTY(Int, componentData, 5999);

  Camera(
    //Int componentData_ = 5999                                             /*** "= 5999" Only needed if you want a default constructor***/
  );
  ~Camera();
  void Init();
  void End();

  std::unique_ptr<Component> Clone() const;
  std::string GetName();

  void UpdateEventsListen(UpdateEvent* updateEvent);
  //void DrawEventsListen(DrawEvent* drawEvent);

  static std::unique_ptr<Camera> RegisterCamera();





  void UpdateViewMat();
  const glm::mat4& GetView() const { return view_; }
  const glm::mat4& GetProj() const { return proj_; }
  const glm::vec3& GetPos() const { return worldpos_; }
  const glm::vec3& GetDir() const { return dir_; }
  const Frustum* GetFrustum() const { return frustum_; }
  float GetFov() const { return fovDeg_; }
  float GetNear() const { return near_; }
  float GetFar() const { return far_; }
  glm::vec3 GetEuler() const { return { pitch_, yaw_, roll_ }; }

  void SetPos(const glm::vec3& v) { worldpos_ = v; UpdateViewMat(); }
  void SetFar(float f) { far_ = f; GenProjection(); }
  void GenProjection(float fovDeg = 80.f)
  {
    proj_ = glm::perspective(glm::radians(fovDeg), 1920.f / 1080.f, near_, far_);
  }
private:

  friend void RegisterComponents();

  Frustum* frustum_;
  glm::vec3 worldpos_ = glm::vec3(150, 50, 100);
  glm::vec3 dir_ = glm::vec3(-.22f, .22f, -.95f);
  glm::mat4 view_ = glm::mat4(1);
  glm::mat4 proj_;

  glm::vec3 up = glm::vec3(0, 1.f, 0);
  glm::vec3 front = glm::vec3(0, 0, -1.f);

  float speed_ = 3.5f;

  float pitch_ = 16;
  float yaw_ = 255;
  float roll_ = 0;
  float fovDeg_ = 80.f;

  float near_ = .1f;
  float far_ = 300.f;
};