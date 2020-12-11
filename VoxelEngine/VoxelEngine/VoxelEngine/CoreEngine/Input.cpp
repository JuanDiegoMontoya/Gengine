#include "EnginePCH.h"
#include "Input.h"
#include <CoreEngine/Engine.h>
#include <functional>
#include <CoreEngine/GAssert.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>

#pragma warning(disable: 26812) // warning telling us to use scoped enums

#define DEBUG_INPUT 0
#if DEBUG_INPUT
#include <iostream>
#endif


void Input::Update()
{
  for (unsigned i = 0; i < BUTTON_COUNT; ++i)
  {
    // keystates decay to either up or down after one frame
    if (keyStates[i] & KeyState::up) keyStates[i]  = KeyState::up;
    if (keyStates[i] & KeyState::down) keyStates[i] = KeyState::down;
  }
  for (unsigned i = 0; i < MOUSE_BUTTON_STATES; i++)
  {
    if (mouseButtonStates[i] & KeyState::up) mouseButtonStates[i] = KeyState::up;
    if (mouseButtonStates[i] & KeyState::down) mouseButtonStates[i] = KeyState::down;
  }
  scrollOffset = glm::vec2(0);
  screenOffset = glm::vec2(0);
  glfwPollEvents();
}

Input::KeyState Input::GetKeyState(Key key)
{
  return keyStates[key];
}
bool Input::IsKeyDown(Key key)
{
  return keyStates[key] & KeyState::down;
}
bool Input::IsKeyUp(Key key)
{
  return keyStates[key] & KeyState::up;
}
bool Input::IsKeyPressed(Key key)
{
  return keyStates[key] == KeyState::pressed;
}
bool Input::IsKeyReleased(Key key)
{
  return keyStates[key] == KeyState::released;
}

bool Input::IsMouseDown(MouseButton key)
{
  return mouseButtonStates[key] & KeyState::down;
}

bool Input::IsMouseUp(MouseButton key)
{
  return mouseButtonStates[key] & KeyState::up;
}

bool Input::IsMousePressed(MouseButton key)
{
  return mouseButtonStates[key] == KeyState::pressed;
}

bool Input::IsMouseReleased(MouseButton key)
{
  return mouseButtonStates[key] == KeyState::released;
}

void Input::keypress_cb([[maybe_unused]] GLFWwindow* window, int key, int scancode, int action, int mods)
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

void Input::mouse_pos_cb([[maybe_unused]] GLFWwindow* window, double xpos, double ypos)
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

void Input::mouse_scroll_cb([[maybe_unused]] GLFWwindow* window, double xoffset, double yoffset)
{
  scrollOffset.x = (float)xoffset;
  scrollOffset.y = (float)yoffset;

#if DEBUG_INPUT
  std::cout << "Mouse scroll: " << "(" << xoffset << ", " << yoffset << ")" << std::endl;
#endif
}

void Input::mouse_button_cb(GLFWwindow* window, int button, int action, int mods)
{
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

  switch (action)
  {
  case GLFW_RELEASE:
    mouseButtonStates[button] = KeyState::released;
    break;
  case GLFW_PRESS:
    mouseButtonStates[button] = KeyState::pressed;
    break;
  case GLFW_REPEAT:
    mouseButtonStates[button] = KeyState::repeat;
    break;
  default:
    ASSERT_MSG(0, "Invalid keycode.");
    break;
  }

#if DEBUG_INPUT
    std::cout << "Mouse button: " << button << std::endl;
#endif
}

// sets GLFW input callbacks
void Input::init_glfw_input_cbs(GLFWwindow* window)
{
  glfwSetKeyCallback(window, keypress_cb);
  glfwSetCursorPosCallback(window, mouse_pos_cb);
  glfwSetScrollCallback(window, mouse_scroll_cb);
  glfwSetMouseButtonCallback(window, mouse_button_cb);
}
