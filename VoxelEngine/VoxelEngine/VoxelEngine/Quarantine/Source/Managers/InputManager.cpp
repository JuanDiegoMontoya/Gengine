/*HEADER_GOES_HERE*/
#include "../../Headers/Managers/InputManager.h"
#include "../../Headers/Factory.h"
#include <ImGuiIncludes.h>
#include <Events/UpdateEvent.h>
#include <Engine.h>
#include <functional>

#pragma warning(disable: 26812) // warning telling us to use scoped enums

#define DEBUG_INPUT 0
#if DEBUG_INPUT
#include <iostream>
#endif

InputManager* InputManager::pInputManager = nullptr;

#pragma optimize("", off)

InputManager::InputManager()
{
}

InputManager::~InputManager()
{
  pInputManager = nullptr;
}

std::string InputManager::GetName()
{
  return "InputManager";
}

void InputManager::Init()
{
  Engine::GetEngine()->RegisterListener(this, &InputManager::UpdateEventsListen);
}

void InputManager::End()
{
  Engine::GetEngine()->UnregisterListener(this, &InputManager::UpdateEventsListen);
}

void InputManager::UpdateEventsListen(UpdateEvent* updateEvent)
{
  for (unsigned i = 0; i < BUTTON_COUNT; ++i)
  {
    // keystates decay to either up or down after one frame
    if (keyStates[i] & KeyState::up) keyStates[i]  = KeyState::up;
    if (keyStates[i] & KeyState::down) keyStates[i] = KeyState::down;
  }
  scrollOffset = glm::vec2(0);
  screenOffset = glm::vec2(0);
  glfwPollEvents();
}

InputManager::KeyState InputManager::GetKeyState(Key key)
{
  return keyStates[key];
}
bool InputManager::IsKeyDown(Key key)
{
  return keyStates[key] & KeyState::down;
}
bool InputManager::IsKeyUp(Key key)
{
  return keyStates[key] & KeyState::up;
}
bool InputManager::IsKeyPressed(Key key)
{
  return keyStates[key] == KeyState::pressed;
}
bool InputManager::IsKeyReleased(Key key)
{
  return keyStates[key] == KeyState::released;
}

void InputManager::keypress_cb([[maybe_unused]] GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key != GLFW_KEY_UNKNOWN)
  {
    switch (action)
    {
    case GLFW_RELEASE:
      keyStates[key] = KeyState::released;
      break;
    case GLFW_PRESS:
      keyStates[key] = KeyState::pressed;
      break;
    case GLFW_REPEAT:
      keyStates[key] = KeyState::repeat;
      break;
    default:
      ASSERT_MSG(0, "Invalid keycode.");
      break;
    }
  }

#if DEBUG_INPUT
  std::cout << "Key pressed: " << (key) << " Action: " << (action) << std::endl;
#endif
}

void InputManager::mouse_pos_cb([[maybe_unused]] GLFWwindow* window, double xpos, double ypos)
{
  static bool firstMouse = true;
  if (firstMouse)
  {
    screenOffset.x = xpos;
    screenOffset.y = ypos;
    firstMouse = false;
  }

  screenPos.x = (float)xpos;
  screenPos.y = (float)ypos;

  worldPos.x = (float)xpos;
  worldPos.y = (float)ypos;

  screenOffset.x = xpos - prevScreenPos.x;
  screenOffset.y = prevScreenPos.y - ypos;
  prevScreenPos = glm::vec2(xpos, ypos);
  screenOffset *= sensitivity;

#if DEBUG_INPUT
  std::cout << "Mouse pos: " << "(" << xpos << ", " << ypos << ")" << std::endl;
#endif
}

void InputManager::mouse_scroll_cb([[maybe_unused]] GLFWwindow* window, double xoffset, double yoffset)
{
  scrollOffset.x = (float)xoffset;
  scrollOffset.y = (float)yoffset;

#if DEBUG_INPUT
  std::cout << "Mouse scroll: " << "(" << xoffset << ", " << yoffset << ")" << std::endl;
#endif
}

void InputManager::mouse_button_cb(GLFWwindow* window, int button, int action, int mods)
{
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

  button += MOUSE_OFFSET; //set to start of mouse buttons in array
  switch (action)
  {
  case GLFW_RELEASE:
    keyStates[button] = KeyState::released;
    break;
  case GLFW_PRESS:
    keyStates[button] = KeyState::pressed;
    break;
  case GLFW_REPEAT:
    keyStates[button] = KeyState::repeat;
    break;
  default:
    ASSERT_MSG(0, "Invalid keycode.");
    break;
  }
}

// sets GLFW input callbacks
void InputManager::init_glfw_input_cbs(GLFWwindow* window)
{
  glfwSetKeyCallback(window, keypress_cb_wrapper);
  glfwSetCursorPosCallback(window, mouse_pos_cb_wrapper);
  glfwSetScrollCallback(window, mouse_scroll_cb_wrapper);
  glfwSetMouseButtonCallback(window, mouse_button_cb_wrapper);
}
