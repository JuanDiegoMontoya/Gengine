#include "../PCH.h"
#include "Logging.h"
#include "../Console.h"
#include "../Parser.h"

#include <shared_mutex>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/chrono.h>

#include <iostream>
#include <chrono>

template<typename Mutex>
class console_sink : public spdlog::sinks::base_sink<Mutex>
{
protected:
  void sink_it_(const spdlog::details::log_msg& msg) final
  {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    Console::Get()->Log(fmt::to_string(formatted).c_str());
  }

  void flush_() final
  {
  }
};

using console_sink_mt = console_sink<std::shared_mutex>;
using console_sink_st = console_sink<spdlog::details::null_mutex>;

using namespace spdlog::sinks;
static std::shared_ptr<basic_file_sink_mt> fileSink = nullptr;
static std::shared_ptr<console_sink_mt> gameConsoleSink = nullptr;
static std::shared_ptr<stdout_color_sink_mt> stdoutSink = nullptr;
static std::shared_ptr<spdlog::logger> defaultLogger = nullptr;

namespace engine::Core
{
  void SetFileSinkLevel(const char* args)
  {
    CmdParser parser(args);
    auto atom = parser.NextAtom();
    if (cvar_float* val = std::get_if<cvar_float>(&atom))
      fileSink->set_level(static_cast<spdlog::level::level_enum>(*val));
    else
      Console::Get()->Log("Usage: SetFileSinkLevel <int>");
  }

  void SetConsoleSinkLevel(const char* args)
  {
    CmdParser parser(args);
    auto atom = parser.NextAtom();
    if (cvar_float* val = std::get_if<cvar_float>(&atom))
      gameConsoleSink->set_level(static_cast<spdlog::level::level_enum>(*val));
    else
      Console::Get()->Log("Usage: SetConsoleSinkLevel <int>");
  }

  void SetStdoutSinkLevel(const char* args)
  {
    CmdParser parser(args);
    auto atom = parser.NextAtom();
    if (cvar_float* val = std::get_if<cvar_float>(&atom))
      stdoutSink->set_level(static_cast<spdlog::level::level_enum>(*val));
    else
      Console::Get()->Log("Usage: SetStdoutSinkLevel <int>");
  }

  void PrintLogLevels(const char*)
  {
    constexpr auto formatStr = R"(TRACE: %d
DEBUG: %d
INFO: %d
WARN: %d
ERROR: %d
CRITICAL: %d
OFF: %d)";
    Console::Get()->Log(formatStr,
      SPDLOG_LEVEL_TRACE, SPDLOG_LEVEL_DEBUG,
      SPDLOG_LEVEL_INFO, SPDLOG_LEVEL_WARN,
      SPDLOG_LEVEL_ERROR, SPDLOG_LEVEL_CRITICAL,
      SPDLOG_LEVEL_OFF);
  }

  void InitLogging()
  {
    std::chrono::system_clock::time_point tp(std::chrono::system_clock::now());
    std::string filename = fmt::format("logs/Log {:%Y-%m-%d %H-%M-%S}.txt", tp);

    fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, false);
    gameConsoleSink = std::make_shared<console_sink_mt>();
    stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    defaultLogger = MakeUniversalLogger("default");
    defaultLogger->set_pattern("[%H:%M:%S:%e] [%n] [%^%l%$] %v");
    defaultLogger->set_level(spdlog::level::debug);
    spdlog::set_default_logger(defaultLogger);

    Console::Get()->RegisterCommand("SetFileSinkLevel", "- Sets the warning level of the file sink.", SetFileSinkLevel);
    Console::Get()->RegisterCommand("SetConsoleSinkLevel", "- Sets the warning level of the console sink.", SetConsoleSinkLevel);
    Console::Get()->RegisterCommand("SetStdoutSinkLevel", "- Sets the warning level of the stdout sink.", SetStdoutSinkLevel);
    Console::Get()->RegisterCommand("PrintLogLevels", "- Prints all logger warning levels.", PrintLogLevels);
  }

  std::shared_ptr<spdlog::logger> MakeUniversalLogger(const std::string& name)
  {
    std::vector<spdlog::sink_ptr> sinks{ gameConsoleSink, stdoutSink, fileSink };
    auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
    spdlog::register_logger(logger);
    return logger;
  }
}