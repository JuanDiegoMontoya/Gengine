#pragma once

#include "Manager.h"
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

class InputManager : public Manager
{
public:

  enum KeyState
  {
    down     = 0b00001,
    pressed  = 0b00011,
    up       = 0b00100,
    released = 0b01100,
    repeat   = 0b10001
  };

  static const ID managerType = cInputManager;

  static InputManager* pInputManager;

  static InputManager* const GetInputManager() { SINGLETON(InputManager, pInputManager); }

  ~InputManager();
  void Init();
  void End();
  std::string GetName();

  void UpdateEventsListen(UpdateEvent* updateEvent);

  glm::vec2 GetScreenPos() { return screenPos; }
  glm::vec2 GetWorldPos() { return worldPos; }
  glm::vec2 GetScreenOffset() { return screenOffset; }
  glm::vec2 GetPrevScreenPos() { return prevScreenPos; }
  glm::vec2 GetScrollOffset() { return scrollOffset; }

  InputManager::KeyState GetKeyState(Key key);
  bool IsKeyDown(Key key);
  bool IsKeyUp(Key key);
  bool IsKeyPressed(Key key);
  bool IsKeyReleased(Key key);

  void init_glfw_input_cbs(GLFWwindow* window);

  float sensitivity = 0.05f;

private:
  InputManager();

  friend void RegisterManagers();

  glm::vec2 screenPos, worldPos;
  glm::vec2 screenOffset, prevScreenPos; // movement since last frame
  glm::vec2 scrollOffset;

  void keypress_cb    (GLFWwindow* window, int key, int scancode, int action, int mods);
  void mouse_pos_cb   (GLFWwindow* window, double xpos, double ypos);
  void mouse_scroll_cb(GLFWwindow* window, double xoffset, double yoffset);
  void mouse_button_cb(GLFWwindow* window, int button, int action, int mods);

  static void keypress_cb_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods) { InputManager::GetInputManager()->keypress_cb(window, key, scancode, action, mods); }
  static void mouse_pos_cb_wrapper(GLFWwindow* window, double xpos, double ypos) { InputManager::GetInputManager()->mouse_pos_cb(window, xpos, ypos); }
  static void mouse_scroll_cb_wrapper(GLFWwindow* window, double xoffset, double yoffset) { InputManager::GetInputManager()->mouse_scroll_cb(window, xoffset, yoffset); }
  static void mouse_button_cb_wrapper(GLFWwindow* window, int button, int action, int mods) { InputManager::GetInputManager()->mouse_button_cb(window, button, action, mods); }

  InputManager::KeyState keyStates[BUTTON_COUNT] = { InputManager::KeyState(0) };
};

inline bool IsKeyDown(Key key)
{
  return InputManager::GetInputManager()->IsKeyDown(key);
}
inline bool IsKeyUp(Key key)
{
  return InputManager::GetInputManager()->IsKeyUp(key);
}
inline bool IsKeyPressed(Key key)
{
  return InputManager::GetInputManager()->IsKeyPressed(key);
}
inline bool IsKeyReleased(Key key)
{
  return InputManager::GetInputManager()->IsKeyDown(key);
}