#pragma once

#include "System.h"
#include "../FactoryID.h"
#include <Systems/Graphics/GraphicsIncludes.h>
#include <functional>

class UpdateEvent;

using Key = unsigned;

#define KeyIsDown( KEY ) InputManager::GetInputManager()->IsKeyDown( KEY )
#define KeyIsUp( KEY ) InputManager::GetInputManager()->IsKeyUp( KEY )
#define KeyIsPressed( KEY ) InputManager::GetInputManager()->IsKeyPressed( KEY )
#define KeyIsReleased( KEY ) InputManager::GetInputManager()->IsKeyReleased( KEY )

#define MOUSE_OFFSET GLFW_KEY_MENU
#define MOUSE_COUNT GLFW_MOUSE_BUTTON_LAST

#define GAMEPAD_OFFSET MOUSE_OFFSET + MOUSE_COUNT
#define GAMEPAD_COUNT GLFW_GAMEPAD_BUTTON_LAST

#define BUTTON_COUNT GAMEPAD_OFFSET + GAMEPAD_COUNT

class InputSystem : public System
{
public:

	enum struct KeyState
	{
		up,
		down,
		pressed,
		released
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

	void init_glfw_input_cbs(struct GLFWwindow* window);

	float sensitivity = 0.05f;
private:
	InputSystem();

	friend void RegisterSystems();

	glm::vec2 screenPos, worldPos;
	glm::vec2 screenOffset, prevScreenPos; // movement since last frame
	glm::vec2 scrollOffset;

	void keypress_cb		([[maybe_unused]] GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouse_pos_cb		([[maybe_unused]] GLFWwindow* window, double xpos, double ypos);
	void mouse_scroll_cb([[maybe_unused]] GLFWwindow* window, double xoffset, double yoffset);
	void mouse_button_cb([[maybe_unused]] GLFWwindow* window, int button, int action, int mods);

	std::function<void(GLFWwindow*, int, int, int, int)>	keypress_cb_bound;
	std::function<void(GLFWwindow*, double, double)>			mouse_pos_cb_bound;
	std::function<void(GLFWwindow*, double, double)>			mouse_scroll_cb_bound;
	std::function<void(GLFWwindow*, int, int, int)>				mouse_button_cb_bound;

	void(*keypress_cb_ptr)		(GLFWwindow*, int, int, int, int);
	void(*mouse_pos_cb_ptr)		(GLFWwindow*, double, double);
	void(*mouse_scroll_cb_ptr)(GLFWwindow*, double, double);
	void(*mouse_button_cb_ptr)(GLFWwindow*, int, int, int);

	InputSystem::KeyState keyStates[BUTTON_COUNT] = { InputSystem::KeyState(0) };
};
