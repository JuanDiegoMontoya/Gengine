/*HEADER_GOES_HERE*/
#include <CoreEngine/Camera.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Frustum.h>
#include <iostream>
#include <CoreEngine/utilities.h>


Camera::Camera()
{
  frustum_ = std::make_unique<Frustum>();
  GenProjection();
}

Camera::~Camera()
{
}

void Camera::Update(float dt)
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

  UpdateViewMat();

  //printf("%f, %f, %f\n", front.x, front.y, front.z);
  //printf("%f, %f, %f\n", worldpos_.x, worldpos_.y, worldpos_.z);
}

void Camera::UpdateViewMat()
{
  view_ = glm::lookAt(worldpos_, worldpos_ + front, up);
  frustum_->Transform(proj_, view_);
  dirty_ = false;
}

void Camera::GenProjection(float fovDeg)
{
  //proj_ = glm::perspective(glm::radians(fovDeg), 1920.f / 1080.f, near_, far_);
  proj_ = Utils::MakeInfReversedZProjRH(glm::radians(fovDeg), 1920.f / 1080.f, near_);
  dirty_ = true;
}
