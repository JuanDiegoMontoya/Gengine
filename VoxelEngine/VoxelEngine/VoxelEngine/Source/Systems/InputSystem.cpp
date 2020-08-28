/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/InputSystem.h"
#include "../../Headers/Factory.h"
#include <ImGuiIncludes.h>
#include <Events/UpdateEvent.h>
#include <Engine.h>

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
	Input::update();
}


















using namespace std;

namespace Input
{
	static mouse_input mouse;
	static kb_input keyboard;

	const mouse_input& Mouse()
	{
		return mouse;
	}

	const kb_input& Keyboard()
	{
		return keyboard;
	}

	static void keypress_cb([[maybe_unused]] GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key != GLFW_KEY_UNKNOWN)
		{
			switch (action)
			{
			case GLFW_RELEASE:
				keyboard.released[key] = true;
				keyboard.down[key] = false;
				break;
			case GLFW_PRESS: // fall-through intentional
				keyboard.pressed[key] = true;
			case GLFW_REPEAT:
				keyboard.down[key] = true;
				break;
			default:
				ASSERT_MSG(0, "Invalid keycode.");
				break;
			}
		}

		cout << "Key pressed: " << (key) << " Action: " << (action) << endl;
	}

	static bool firstMouse = true;
	static void mouse_pos_cb([[maybe_unused]] GLFWwindow* window, double xpos, double ypos)
	{
		if (firstMouse)
		{
			mouse.screenOffset.x = xpos;
			mouse.screenOffset.y = ypos;
			firstMouse = false;
		}

		mouse.screenPos.x = (float)xpos;
		mouse.screenPos.y = (float)ypos;

		mouse.worldPos.x = (float)xpos;
		mouse.worldPos.y = (float)ypos;

		mouse.screenOffset.x = xpos - mouse.prevScreenPos.x;
		mouse.screenOffset.y = mouse.prevScreenPos.y - ypos;
		mouse.prevScreenPos = glm::vec2(xpos, ypos);
		mouse.screenOffset *= mouse.sensitivity;
		cout << "Mouse pos: " << "(" << xpos << ", " << ypos << ")" << endl;
	}

	static void mouse_scroll_cb([[maybe_unused]] GLFWwindow* window, double xoffset, double yoffset)
	{
		mouse.scrollOffset.x = (float)xoffset;
		mouse.scrollOffset.y = (float)yoffset;

		cout << "Mouse scroll: " << "(" << xoffset << ", " << yoffset << ")" << endl;
	}

	static void mouse_button_cb(GLFWwindow* window, int button, int action, int mods)
	{
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

		switch (action)
		{
		case GLFW_RELEASE:
			mouse.released[button] = true;
			mouse.down[button] = false;
			break;
		case GLFW_PRESS: // fall-through intentional
			mouse.pressed[button] = true;
		case GLFW_REPEAT:
			mouse.down[button] = true;
			break;
		default:
			ASSERT_MSG(0, "Invalid keycode.");
			break;
		}

		cout << "Mouse clicked: " << (button) << " Action: " << (action) << endl;
	}

	// sets GLFW input callbacks
	void init_glfw_input_cbs(GLFWwindow* window)
	{
		glfwSetKeyCallback(window, keypress_cb);
		glfwSetMouseButtonCallback(window, mouse_button_cb);
		glfwSetScrollCallback(window, mouse_scroll_cb);
		glfwSetCursorPosCallback(window, mouse_pos_cb);
	}

	// clears temporary input (presses and releases) and polls for new events
	void update()
	{
		for (int i = 0; i < GLFW_KEY_LAST; i++)
		{
			keyboard.pressed[i] = false;
			keyboard.released[i] = false;
		}

		for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
		{
			mouse.pressed[i] = false;
			mouse.released[i] = false;
		}

		mouse.scrollOffset = glm::vec2(0);
		mouse.screenOffset = glm::vec2(0);
		glfwPollEvents();
	}
}