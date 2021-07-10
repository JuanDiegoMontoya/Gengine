#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <Utilities/HashedString.h>
#include <span>
#include <variant>
#include <unordered_map>

//constexpr int MOUSE_OFFSET = GLFW_KEY_MENU;
//constexpr int MOUSE_COUNT = GLFW_MOUSE_BUTTON_LAST;

//constexpr int GAMEPAD_OFFSET = MOUSE_OFFSET + MOUSE_COUNT;
//constexpr int GAMEPAD_COUNT = GLFW_GAMEPAD_BUTTON_LAST;
//
//constexpr int BUTTON_COUNT = GAMEPAD_OFFSET + GAMEPAD_COUNT;
constexpr int BUTTON_COUNT = GLFW_KEY_LAST;
constexpr int MOUSE_BUTTON_STATES = GLFW_MOUSE_BUTTON_LAST;

class UpdateEvent;
struct GLFWwindow;

// keycodes can be negative in case of an error
using Key = int;
using MouseButton = int;

struct InputKey { Key key{}; };
struct InputMouseButton { MouseButton button; };
struct InputMouseScroll { bool yaxis{ false }; };
struct InputMousePos { bool yaxis{ false }; };

using InputActionType = std::variant<InputKey, InputMouseButton, InputMouseScroll>;
using InputAxis_t = std::variant<InputKey, InputMouseButton, InputMouseScroll, InputMousePos>;
struct InputAxisType { float scale{ 1 }; InputAxis_t type; };

class Input
{
public:
  enum KeyState : uint8_t // applicable to keyboard keys and mouse buttons
  {
    down     = 0b00001,
    pressed  = 0b00011,
    up       = 0b00100,
    released = 0b01100,
    repeat   = 0b10001
  };

  static void Update();

  static glm::vec2 GetScreenPos() { return screenPos; }
  static glm::vec2 GetScreenOffset() { return screenOffset; }
  static glm::vec2 GetPrevScreenPos() { return prevScreenPos; }
  static glm::vec2 GetScrollOffset() { return scrollOffset; }

  static KeyState GetKeyState(Key key);
  static bool IsKeyDown(Key key);
  static bool IsKeyUp(Key key);
  static bool IsKeyPressed(Key key);
  static bool IsKeyReleased(Key key);
  static bool IsMouseDown(MouseButton key);
  static bool IsMouseUp(MouseButton key);
  static bool IsMousePressed(MouseButton key);
  static bool IsMouseReleased(MouseButton key);

  static void init_glfw_input_cbs(GLFWwindow* window);

  static inline float sensitivity = 0.05f;

  static void AddInputAction(hashed_string action, std::span<InputActionType> keys);
  static void RemoveInputAction(hashed_string action);
  static bool IsInputActionPressed(hashed_string action);

  static void AddInputAxis(hashed_string action, std::span<InputAxisType> keys);
  static void RemoveInputAxis(hashed_string action);
  static float GetInputAxis(hashed_string action);

  static void SetCursorVisible(bool state);
  static bool GetCursorVisible();

  static inline GLFWwindow* window_;

private:
  static inline glm::vec2 screenPos;
  static inline glm::vec2 screenOffset;
  static inline glm::vec2 prevScreenPos;
  static inline glm::vec2 scrollOffset;

  static void keypress_cb(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void mouse_pos_cb(GLFWwindow* window, double xpos, double ypos);
  static void mouse_scroll_cb(GLFWwindow* window, double xoffset, double yoffset);
  static void mouse_button_cb(GLFWwindow* window, int button, int action, int mods);

  static inline KeyState keyStates[BUTTON_COUNT] = { KeyState(0) };
  static inline KeyState mouseButtonStates[MOUSE_BUTTON_STATES] = { KeyState(0) };

  // an action can be mapped to multiple keys
  static inline std::unordered_multimap<uint32_t, InputActionType> inputActions;
  static inline std::unordered_multimap<uint32_t, InputAxisType> inputAxes;
};