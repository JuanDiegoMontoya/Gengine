#pragma once
#include "MathIncludes.h"
#include <memory>
#include <vector>
#include <CoreEngine/Components.h>
#include <CoreEngine/Frustum.h>

//struct Camera;

class Frustum;

class CameraSystem
{
public:
	static void Init();
  //~CameraSystem();

  static void Update(float dt);

  static void RegisterCamera(Components::Camera* cam);

  static const std::vector<Components::Camera*>& GetCameraList()
  {
	  return cameraList;
  }

  static void UpdateViewMat();
  static void GenProjection();

  static const glm::mat4& GetView() { return view_; }
  static const glm::mat4& GetProj() { return proj_; }
  static const glm::mat4& GetViewProj() { return viewProj_; }
  static const glm::vec3& GetPos() { return worldpos_; }
  static const glm::vec3& GetDir() { return dir_; }
  static const Frustum* GetFrustum() { return frustum_.get(); }
  static float GetFov() { return fovDeg_; }
  static float GetNear() { return near_; }
  static float GetFar() { return far_; }
  static glm::vec3 GetEuler() { return { pitch_, yaw_, roll_ }; }
  static auto GetFront() { return front; }
  static auto GetUp() { return up; }

  static void SetPos(const glm::vec3& v) { worldpos_ = v; dirty_ = true; }
  static void SetFar(float f) { far_ = f; GenProjection(); dirty_ = true; }
  static void SetFront(const glm::vec3& f) { front = f; dirty_ = true; }
  static void SetDir(const glm::vec3& v) { dir_ = v; dirty_ = true; }
  static void SetYaw(float f) { yaw_ = f; dirty_ = true; }
  static void SetPitch(float f) { pitch_ = f; dirty_ = true; }

  static inline Components::Camera* ActiveCamera = nullptr;

private:

	static inline std::vector<Components::Camera*> cameraList;

	static inline glm::vec3 worldpos_ = glm::vec3(150, 50, 100);
	static inline glm::vec3 dir_ = glm::vec3(-.22f, .22f, -.95f);

	static inline glm::vec3 up = glm::vec3(0, 1.f, 0);
	static inline glm::vec3 front = glm::vec3(0, 0, -1.f);
	static inline glm::vec3 right = glm::vec3(1, 0, 0);


	static inline float speed_ = 3.5f;

	// view matrix info
	static inline float pitch_ = 16;
	static inline float yaw_ = 255;
	static inline float roll_ = 0;


  // projection matrix info
	static inline float fovDeg_ = 80.f;
	static inline float near_ = .1f;
  static inline float far_ = 300.f;

  static inline glm::mat4 view_ = glm::mat4(1);
  static inline glm::mat4 proj_{};
  static inline glm::mat4 viewProj_{}; // cached
  static inline std::unique_ptr<Frustum> frustum_;

  static inline bool dirty_ = true;
};