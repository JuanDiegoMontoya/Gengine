#pragma once

#include "System.h"
#include "../FactoryID.h"
#include <Graphics/GraphicsIncludes.h>
#include <functional>

constexpr int MOUSE_OFFSET = GLFW_KEY_MENU;
constexpr int MOUSE_COUNT = GLFW_MOUSE_BUTTON_LAST;

constexpr int GAMEPAD_OFFSET = MOUSE_OFFSET + MOUSE_COUNT;
constexpr int GAMEPAD_COUNT = GLFW_GAMEPAD_BUTTON_LAST;

constexpr int BUTTON_COUNT = GAMEPAD_OFFSET + GAMEPAD_COUNT;

class UpdateEvent;
struct GLFWwindow;

// keycodes can be negative in case of an error
using Key = int;

class InputSystem : public System
{
public:

  enum KeyState
  {
    down     = 0b0001,
    pressed  = 0b0011,
    up       = 0b0100,
    released = 0b1100
  };

  static const ID systemType = cInputSystem;

  static InputSystem* pInputSystem;

  static InputSystem* const GetInputSystem() { SINGLETON(InputSystem, pInputSystem); }

  ~InputSystem();
  void Init();
  void End();
  std::string GetName();

  void UpdateEventsListen(UpdateEvent* updateEvent);

  glm::vec2 GetScreenPos() { return screenPos; }
  glm::vec2 GetWorldPos() { return worldPos; }
  glm::vec2 GetScreenOffset() { return screenOffset; }
  glm::vec2 GetPrevScreenPos() { return prevScreenPos; }
  glm::vec2 GetScrollOffset() { return scrollOffset; }

  InputSystem::KeyState GetKeyState(Key key);
  bool IsKeyDown(Key key);
  bool IsKeyUp(Key key);
  bool IsKeyPressed(Key key);
  bool IsKeyReleased(Key key);

  void init_glfw_input_cbs(GLFWwindow* window);

  float sensitivity = 0.05f;

private:
  InputSystem();

  friend void RegisterSystems();

  glm::vec2 screenPos, worldPos;
  glm::vec2 screenOffset, prevScreenPos; // movement since last frame
  glm::vec2 scrollOffset;

  void keypress_cb    (GLFWwindow* window, int key, int scancode, int action, int mods);
  void mouse_pos_cb   (GLFWwindow* window, double xpos, double ypos);
  void mouse_scroll_cb(GLFWwindow* window, double xoffset, double yoffset);
  void mouse_button_cb(GLFWwindow* window, int button, int action, int mods);

  static void keypress_cb_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods) { InputSystem::GetInputSystem()->keypress_cb(window, key, scancode, action, mods); }
  static void mouse_pos_cb_wrapper(GLFWwindow* window, double xpos, double ypos) { InputSystem::GetInputSystem()->mouse_pos_cb(window, xpos, ypos); }
  static void mouse_scroll_cb_wrapper(GLFWwindow* window, double xoffset, double yoffset) { InputSystem::GetInputSystem()->mouse_scroll_cb(window, xoffset, yoffset); }
  static void mouse_button_cb_wrapper(GLFWwindow* window, int button, int action, int mods) { InputSystem::GetInputSystem()->mouse_button_cb(window, button, action, mods); }

  InputSystem::KeyState keyStates[BUTTON_COUNT] = { InputSystem::KeyState(0) };
};

inline bool IsKeyDown(Key key)
{
  return InputSystem::GetInputSystem()->IsKeyDown(key);
}
inline bool IsKeyUp(Key key)
{
  return InputSystem::GetInputSystem()->IsKeyUp(key);
}
inline bool IsKeyPressed(Key key)
{
  return InputSystem::GetInputSystem()->IsKeyDown(key);
}
inline bool IsKeyReleased(Key key)
{
  return InputSystem::GetInputSystem()->IsKeyDown(key);
}