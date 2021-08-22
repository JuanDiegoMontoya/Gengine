#pragma once
#include <memory>
#include <spdlog/spdlog.h>

namespace engine::Core
{
  void InitLogging();
  std::shared_ptr<spdlog::logger> MakeUniversalLogger(const std::string& name);
}