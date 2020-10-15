#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
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

class Input
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

  static void Update();

  static glm::vec2 GetScreenPos() { return screenPos; }
  static glm::vec2 GetWorldPos() { return worldPos; }
  static glm::vec2 GetScreenOffset() { return screenOffset; }
  static glm::vec2 GetPrevScreenPos() { return prevScreenPos; }
  static glm::vec2 GetScrollOffset() { return scrollOffset; }

  static KeyState GetKeyState(Key key);
  static bool IsKeyDown(Key key);
  static bool IsKeyUp(Key key);
  static bool IsKeyPressed(Key key);
  static bool IsKeyReleased(Key key);

  static void init_glfw_input_cbs(GLFWwindow* window);

  static inline float sensitivity = 0.05f;

private:
  static inline glm::vec2 screenPos;
  static inline glm::vec2 worldPos;
  static inline glm::vec2 screenOffset;
  static inline glm::vec2 prevScreenPos;
  static inline glm::vec2 scrollOffset;

  static void keypress_cb    (GLFWwindow* window, int key, int scancode, int action, int mods);
  static void mouse_pos_cb   (GLFWwindow* window, double xpos, double ypos);
  static void mouse_scroll_cb(GLFWwindow* window, double xoffset, double yoffset);
  static void mouse_button_cb(GLFWwindow* window, int button, int action, int mods);

  static inline KeyState keyStates[BUTTON_COUNT] = { KeyState(0) };
};