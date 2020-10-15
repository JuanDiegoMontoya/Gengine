#pragma once

#include <iostream>

#include "WindowUtils.h"

struct GLFWwindow;

class Window
{
public:
	Window(const char* pName, Settings pSettings, Layout pLayout);
	~Window();

	bool Focus();

	GLFWwindow* GetGLFWWindow() { return window; }

	Layout GetLayout() { return layout; }
	Layout& GetLayoutRef() { return layout; }

	bool IsDirtyLayout() { return layout.dirty; }
	bool IsDirtySettings() { return settings.dirty; }

	static inline GLFWwindow* window;

	static inline Settings settings;
	static inline Layout layout;

private:
	std::string name;

	bool WindowInitialized = false;
	bool DirtyLayout = false;

	void MakeWindow();

};