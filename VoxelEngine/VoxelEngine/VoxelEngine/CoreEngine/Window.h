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

	bool IsDirtyLayout() { return DirtyLayout; }

private:
	std::string name;

	bool WindowInitialized = false;
	bool DirtyLayout = false;

	Settings settings;
	Layout layout;

	void MakeWindow();

protected:
	GLFWwindow* window = nullptr;

};