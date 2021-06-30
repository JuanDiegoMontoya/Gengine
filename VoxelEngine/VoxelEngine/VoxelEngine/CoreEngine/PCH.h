#pragma once
// PCH

// std lib
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <optional>
#include <functional>
#include <execution>
#include <type_traits>
#include <variant>

#include <cstdlib>
#include <cstdio>

// vendor
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/integer.hpp>

#include <Utilities/HashedString.h>
#include "GAssert.h"

constexpr inline std::string_view AssetDir = "./Resources/";
constexpr inline std::string_view TextureDir = "./Resources/Textures/";
constexpr inline std::string_view ShaderDir = "./Resources/Shaders/";
constexpr inline std::string_view ModelDir = "./Resources/Models/";