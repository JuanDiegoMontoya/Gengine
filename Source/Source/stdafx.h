#pragma once

// debug flag for advanced debugging
#define DE_BUG 1

// OpenGL API stuff
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GL math libraries
//#define GLM_SWIZZLE_XYZW
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/integer.hpp>
//#include "utilities.h"

// imgui stuff
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

// Lua scripting
#include <lua.hpp>

// common std DS & A
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <concurrent_unordered_map.h>
#include <algorithm>
#include <chrono>
using namespace std::chrono;

// debugging
#include "engine_assert.h"
#include "debug.h"

// other common includes
#include <iostream>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <optional>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#include <TracyOpenGL.hpp>
#endif // TRACY_ENABLE