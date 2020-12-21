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
  // TEMPORARY flying controls until real controls are added
  //float currSpeed = speed_ * dt;
  //if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
  //  currSpeed *= 10;
  //if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL))
  //  currSpeed /= 10;
  //if (Input::IsKeyDown(GLFW_KEY_W))
  //  worldpos_ += currSpeed * front;
  //if (Input::IsKeyDown(GLFW_KEY_S))
  //  worldpos_ -= currSpeed * front;
  //if (Input::IsKeyDown(GLFW_KEY_A))
  //  worldpos_ -= glm::normalize(glm::cross(front, up)) * currSpeed;
  //if (Input::IsKeyDown(GLFW_KEY_D))
  //  worldpos_ += glm::normalize(glm::cross(front, up)) * currSpeed;

  //yaw_ += Input::GetScreenOffset().x;
  //pitch_ += Input::GetScreenOffset().y;

  //pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);

  //glm::vec3 temp;
  //temp.x = cos(glm::radians(pitch_)) * cos(glm::radians(yaw_));
  //temp.y = sin(glm::radians(pitch_));
  //temp.z = cos(glm::radians(pitch_)) * sin(glm::radians(yaw_));
  //front = glm::normalize(temp);
  //dir_ = front;

	Entity entity = ActiveCamera->entity;

	Transform& tr = entity.GetComponent<Components::Transform>();

	// View Setup
	worldpos_ = tr.GetTranslation() + ActiveCamera->translation;

	//glm::vec3 temp;
	//front.x = cos(glm::radians(ActiveCamera->pitch_)) * cos(glm::radians(ActiveCamera->yaw_));
	//front.y = sin(glm::radians(ActiveCamera->pitch_));
	//front.z = cos(glm::radians(ActiveCamera->pitch_)) * sin(glm::radians(ActiveCamera->yaw_));

  front = ActiveCamera->GetForward();

	//front = ActiveCamera->GetForward();
  up = glm::vec3(0, 1, 0);// ActiveCamera->GetUp();
	right = ActiveCamera->GetRight();

  dir_ = front;// ActiveCamera->GetForward();

	// Projection 
	near_ = ActiveCamera->zNear;
	far_ = ActiveCamera->zFar;
	fovDeg_ = ActiveCamera->fov;

	GenProjection();
  UpdateViewMat();

  // TODO Temp 
  tr.SetRotation(view_);

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
