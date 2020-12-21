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

void Input::keypress_cb([[maybe_unused]] GLFWwindow* window, int key, int scancode, int action, [[maybe_unused]] int mods)
{
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

  ImGuiIO& io = ImGui::GetIO();
  if (action == GLFW_PRESS)
    io.KeysDown[key] = true;
  if (action == GLFW_RELEASE)
    io.KeysDown[key] = false;

  io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
  io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
  io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
  io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

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

  screenPos.x = static_cast<float>(xpos);
  screenPos.y = static_cast<float>(ypos);

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
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
  scrollOffset.x = static_cast<float>(xoffset);
  scrollOffset.y = static_cast<float>(yoffset);

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

void Input::init_glfw_input_cbs(GLFWwindow* window)
{
  glfwSetKeyCallback(window, keypress_cb);
  glfwSetCursorPosCallback(window, mouse_pos_cb);
  glfwSetScrollCallback(window, mouse_scroll_cb);
  glfwSetMouseButtonCallback(window, mouse_button_cb);

  glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
}

void Input::AddInputAction(entt::hashed_string action, std::span<InputActionType> keys)
{
  ASSERT_MSG(inputActions.find(action) == inputActions.end(), "Input action already exists.");
  for (auto key : keys)
  {
    inputActions.insert({ action, key });
  }
}

void Input::RemoveInputAction(entt::hashed_string action)
{
  ASSERT_MSG(inputActions.count(action) > 0, "No input action of that type exists.");
  inputActions.erase(action);
}

bool Input::IsInputActionPressed(entt::hashed_string action)
{
  ASSERT_MSG(inputActions.count(action) > 0, "No input action of that type exists.");
  auto range = inputActions.equal_range(action);
  for (auto it = range.first; it != range.second; it++)
  {
    auto var = it->second;
    if (auto key = std::get_if<InputKey>(&var))
    {
      if (IsKeyPressed(key->key))
      {
        return true;
      }
    }
    if (auto button = std::get_if<InputMouseButton>(&var))
    {
      if (IsMousePressed(button->button))
      {
        return true;
      }
    }
    if (auto scroll = std::get_if<InputMouseScroll>(&var))
    {
      if ((scroll->yaxis && GetScrollOffset().y != 0) || (!scroll->yaxis && GetScrollOffset().x != 0))
      {
        return true;
      }
    }
  }
  return false;
}

void Input::AddInputAxis(entt::hashed_string action, std::span<InputAxisType> keys)
{
  ASSERT_MSG(inputAxes.find(action) == inputAxes.end(), "Input axis already exists.");
  for (auto key : keys)
  {
    inputAxes.insert({ action, key });
  }
}

void Input::RemoveInputAxis(entt::hashed_string action)
{
  ASSERT_MSG(inputAxes.count(action) > 0, "No input axis of that type exists.");
  inputAxes.erase(action);
}

float Input::GetInputAxis(entt::hashed_string action)
{
  ASSERT_MSG(inputAxes.count(action) > 0, "No input action of that type exists.");
  auto range = inputAxes.equal_range(action);
  float maxAbs = 0;
  for (auto it = range.first; it != range.second; it++)
  {
    auto var = it->second;
    auto scale = var.scale;
    if (auto key = std::get_if<InputKey>(&var.type))
    {
      if (IsKeyDown(key->key))
      {
        if (auto val = 1.0f * scale; std::abs(val) > std::abs(maxAbs))
        {
          maxAbs = val;
        }
      }
    }
    if (auto button = std::get_if<InputMouseButton>(&var.type))
    {
      if (IsMouseDown(button->button))
      {
        if (auto val = 1.0f * scale; std::abs(val) > std::abs(maxAbs))
        {
          maxAbs = val;
        }
      }
    }
    if (auto scroll = std::get_if<InputMouseScroll>(&var.type))
    {
      if (scroll->yaxis)
      {
        if (auto off = GetScrollOffset().y * scale; std::abs(off) > std::abs(maxAbs))
        {
          maxAbs = off;
        }
      }
      else
      {
        if (auto off = GetScrollOffset().x * scale; std::abs(off) > std::abs(maxAbs))
        {
          maxAbs = off;
        }
      }
    }
    if (auto mouse = std::get_if<InputMousePos>(&var.type))
    {
      if (mouse->yaxis)
      {
        if (auto off = GetScreenOffset().y * scale; std::abs(off) > std::abs(maxAbs))
        {
          maxAbs = off;
        }
      }
      else
      {
        if (auto off = GetScreenOffset().x * scale; std::abs(off) > std::abs(maxAbs))
        {
          maxAbs = off;
        }
      }
    }
  }
  return maxAbs;
}
