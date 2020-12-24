#include "EnginePCH.h"
#include <CoreEngine/Camera.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Frustum.h>
#include <iostream>
#include <CoreEngine/utilities.h>
#include "Components.h"

using namespace Components;

// Camera Component Constuctor
Camera::Camera(Entity entity_) : entity(entity_)
{
	CameraSystem::RegisterCamera(this);
}

void CameraSystem::Init()
{
  frustum_ = std::make_unique<Frustum>();
}

void CameraSystem::RegisterCamera(Components::Camera* cam)
{
	cameraList.push_back(cam);
}

void CameraSystem::Update(float dt)
{
	// View Setup
	worldpos_ = ActiveCamera->GetWorldPos();
  dir_ = front = ActiveCamera->GetForward();
  up = glm::vec3(0, 1, 0);
	//right = ActiveCamera->GetRight();

	// Projection 
	near_ = ActiveCamera->zNear;
	far_ = ActiveCamera->zFar;
	fovDeg_ = ActiveCamera->fov;

	GenProjection();
  UpdateViewMat();

  // TODO Temp 
  //tr.SetRotation(view_);

  //printf("%f, %f, %f\n", front.x, front.y, front.z);
  //printf("%f, %f, %f\n", worldpos_.x, worldpos_.y, worldpos_.z);
}

void CameraSystem::UpdateViewMat()
{
	view_ = glm::lookAt(worldpos_, worldpos_ + front, { 0,1,0 });
  viewProj_ = proj_ * view_;
  frustum_->Transform(proj_, view_);
  dirty_ = false;
}

void CameraSystem::GenProjection()
{
  //proj_ = glm::perspective(glm::radians(fovDeg), 1920.f / 1080.f, near_, far_);
  proj_ = Utils::MakeInfReversedZProjRH(glm::radians(fovDeg_), 1920.f / 1080.f, near_);
  dirty_ = true;
}
