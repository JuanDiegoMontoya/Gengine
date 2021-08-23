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
#include <span>

#include <cstdlib>
#include <cstdio>

// vendor
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/integer.hpp>

#include <utility/HashedString.h>
#include "GAssert.h"
#include "core/Logging.h"

constexpr inline std::string_view AssetDir = "../../../data/game/";
constexpr inline std::string_view TextureDir = "../../../data/game/Textures/";
constexpr inline std::string_view ShaderDir = "../../../data/game/Shaders/";
constexpr inline std::string_view ModelDir = "../../../data/game/Models/";