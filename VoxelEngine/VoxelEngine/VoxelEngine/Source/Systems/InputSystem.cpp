/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/InputSystem.h"
#include "../../Headers/Factory.h"
#include <ImGuiIncludes.h>
#include <Events/UpdateEvent.h>
#include <Engine.h>
#include <functional>

#define DEBUG_INPUT 0

InputSystem* InputSystem::pInputSystem = nullptr;

#pragma optimize("", off)

InputSystem::InputSystem()
{
}

InputSystem::~InputSystem()
{
  pInputSystem = nullptr;
}

std::string InputSystem::GetName()
{
  return "InputSystem";
}

void InputSystem::Init()
{
	Engine::GetEngine()->RegisterListener(this, &InputSystem::UpdateEventsListen);
}

void InputSystem::End()
{
	Engine::GetEngine()->UnregisterListener(this, &InputSystem::UpdateEventsListen);
}

void InputSystem::UpdateEventsListen(UpdateEvent* updateEvent)
{
	for (unsigned i = 0; i < BUTTON_COUNT; ++i)
	{
		if (keyStates[i] == KeyState::pressed) keyStates[i]	= KeyState::up;
		if (keyStates[i] == KeyState::released) keyStates[i] = KeyState::down;
		//if (keyIsDown && keyStates[i] == KeyState::pressed)
		//	keyStates[i] = KeyState::down;
		//else if (keyIsDown && keyStates[i] != KeyState::down)
		//	keyStates[i] = KeyState::pressed;
		//else if (!keyIsDown && keyStates[i] == KeyState::released)
		//	keyStates[i] = KeyState::up;
		//else if (!keyIsDown && keyStates[i] != KeyState::up)
		//	keyStates[i] = KeyState::released;
	}
	scrollOffset = glm::vec2(0);
	screenOffset = glm::vec2(0);
	glfwPollEvents();
}

InputSystem::KeyState InputSystem::GetKeyState(Key key)
{
	return keyStates[static_cast<unsigned>(key)];
}
bool InputSystem::IsKeyDown(Key key)
{
	return keyStates[static_cast<unsigned>(key)] == KeyState::down
		|| keyStates[static_cast<unsigned>(key)] == KeyState::pressed;
}
bool InputSystem::IsKeyUp(Key key)
{
	return keyStates[static_cast<unsigned>(key)] == KeyState::up
		|| keyStates[static_cast<unsigned>(key)] == KeyState::released;
}
bool InputSystem::IsKeyPressed(Key key)
{
	return keyStates[static_cast<unsigned>(key)] == KeyState::pressed;
}
bool InputSystem::IsKeyReleased(Key key)
{
	return keyStates[static_cast<unsigned>(key)] == KeyState::released;
}

void InputSystem::keypress_cb([[maybe_unused]] GLFWwindow* window, int key, int scancode, int action, int mods)
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
			keyStates[key] = KeyState::down;
			break;
		default:
			ASSERT_MSG(0, "Invalid keycode.");
			break;
		}
	}

#if DEBUG_INPUT
	cout << "Key pressed: " << (key) << " Action: " << (action) << endl;
#endif
}

void InputSystem::mouse_pos_cb([[maybe_unused]] GLFWwindow* window, double xpos, double ypos)
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
	cout << "Mouse pos: " << "(" << xpos << ", " << ypos << ")" << endl;
#endif
}

void InputSystem::mouse_scroll_cb([[maybe_unused]] GLFWwindow* window, double xoffset, double yoffset)
{
	scrollOffset.x = (float)xoffset;
	scrollOffset.y = (float)yoffset;

#if DEBUG_INPUT
	cout << "Mouse scroll: " << "(" << xoffset << ", " << yoffset << ")" << endl;
#endif
}

void InputSystem::mouse_button_cb(GLFWwindow* window, int button, int action, int mods)
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
		keyStates[button] = KeyState::down;
		break;
	default:
		ASSERT_MSG(0, "Invalid keycode.");
		break;
	}
}
// sets GLFW input callbacks
void InputSystem::init_glfw_input_cbs(GLFWwindow* window)
{
	using namespace std::placeholders;
	keypress_cb_bound			= std::bind(&InputSystem::keypress_cb,			this, _1, _2, _3, _4, _5);
	mouse_pos_cb_bound		= std::bind(&InputSystem::mouse_pos_cb,		this, _1, _2, _3);
	mouse_scroll_cb_bound = std::bind(&InputSystem::mouse_scroll_cb, this, _1, _2, _3);
	mouse_button_cb_bound = std::bind(&InputSystem::mouse_button_cb, this, _1, _2, _3, _4);
	
	keypress_cb_ptr			= keypress_cb_bound.target<void(GLFWwindow*, int, int, int, int)>();
	mouse_pos_cb_ptr		= mouse_pos_cb_bound.target<void(GLFWwindow*, double, double)>();
	mouse_scroll_cb_ptr = mouse_scroll_cb_bound.target<void(GLFWwindow*, double, double)>();
	mouse_button_cb_ptr = mouse_button_cb_bound.target<void(GLFWwindow*, int, int, int)>();

	glfwSetKeyCallback(window, keypress_cb_ptr);
	glfwSetCursorPosCallback(window, mouse_scroll_cb_ptr);
	glfwSetScrollCallback(window, mouse_scroll_cb_ptr);
	glfwSetMouseButtonCallback(window, mouse_button_cb_ptr);
}
