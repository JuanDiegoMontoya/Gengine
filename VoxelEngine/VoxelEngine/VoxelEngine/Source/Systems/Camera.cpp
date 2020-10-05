/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/Camera.h"
#include "../../Headers/Containers/Object.h"
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Factory.h"

#include "../../Headers/Events/UpdateEvent.h"
//#include "../../Headers/Events/DrawEvent.h"
#include <Managers/InputManager.h>
#include <Rendering/Frustum.h>

std::string Camera::GetName() { return "Camera"; }

std::unique_ptr<Camera> Camera::RegisterCamera() 
{
  auto system = std::unique_ptr<Camera>(new Camera(
    //5999
		/***Only needed if you don't provide a default constructor***/
  ));

  return std::move(system);
}

Camera::Camera(
  //Int systemData_
) : System(systemType) //, systemData(systemData_)
{
	frustum_ = std::make_unique<Frustum>();
}

Camera::~Camera()
{
}

void Camera::Init()
{
  GetSpace()->RegisterListener(this, &Camera::UpdateEventsListen);
  //GetSpace()->RegisterListener(this, &Camera::DrawEventsListen);
}

void Camera::End()
{
  if (parent != nullptr)
  {
    GetSpace()->UnregisterListener(this, &Camera::UpdateEventsListen);
  //  GetSpace()->UnregisterListener(this, &Camera::DrawEventsListen);
  }
}

std::unique_ptr<System> Camera::Clone() const
{
  auto result = new Camera();
    //copy over values here
  //result->systemData = systemData;

  return std::unique_ptr<System>(result);
}

void Camera::UpdateEventsListen(UpdateEvent* updateEvent)
{
	// TEMPORARY flying controls until real controls are added
	float currSpeed = speed_ * updateEvent->dt;
	if (IsKeyDown(GLFW_KEY_LEFT_SHIFT))
		currSpeed *= 10;
	if (IsKeyDown(GLFW_KEY_LEFT_CONTROL))
		currSpeed /= 10;
	if (IsKeyDown(GLFW_KEY_W))
		worldpos_ += currSpeed * front;
	if (IsKeyDown(GLFW_KEY_S))
		worldpos_ -= currSpeed * front;
	if (IsKeyDown(GLFW_KEY_A))
		worldpos_ -= glm::normalize(glm::cross(front, up)) * currSpeed;
	if (IsKeyDown(GLFW_KEY_D))
		worldpos_ += glm::normalize(glm::cross(front, up)) * currSpeed;

	yaw_ += InputManager::GetInputManager()->GetScreenOffset().x;
	pitch_ += InputManager::GetInputManager()->GetScreenOffset().y;

	pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);

	glm::vec3 temp;
	temp.x = cos(glm::radians(pitch_)) * cos(glm::radians(yaw_));
	temp.y = sin(glm::radians(pitch_));
	temp.z = cos(glm::radians(pitch_)) * sin(glm::radians(yaw_));
	front = glm::normalize(temp);
	dir_ = front;

	UpdateViewMat();

	//printf("%f, %f, %f\n", front.x, front.y, front.z);
	//printf("%f, %f, %f\n", worldpos_.x, worldpos_.y, worldpos_.z);
}

//void Camera::DrawEventsListen(DrawEvent* drawEvent) { }


void Camera::UpdateViewMat()
{
  view_ = glm::lookAt(worldpos_, worldpos_ + front, up);
  frustum_->Transform(proj_, view_);
}

void Camera::GenProjection(float fovDeg)
{
	proj_ = glm::perspective(glm::radians(fovDeg), 1920.f / 1080.f, near_, far_);
}
