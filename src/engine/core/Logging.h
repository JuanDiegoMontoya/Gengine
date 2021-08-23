#pragma once
#include <memory>

#ifdef _WIN32 // I hate Windows.h
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
  #define NOGDICAPMASKS
  #define NOCRYPT
  #define NOVIRTUALKEYCODES
  #define NOWINMESSAGES
  #define NOWINSTYLES
  #define NOSYSMETRICS
  #define NOMENUS
  #define NOICONS
  #define NOKEYSTATES
  #define NORASTEROPS
  #define NOSYSCOMMANDS
  #define NOSHOWWINDOW
  #define OEMRESOURCE
  #define NOATOM
  #define NOCLIPBOARD
  #define NOCOLOR
  #define NOCTLMGR
  #define NODRAWTEXT
  #define NOGDI
  #define NOKERNEL
  #define NOUSER
  #define NONLS
  #define NOMB
  #define NOMEMMGR
  #define NOMETAFILE
  #define NOMSG
  #define NOOPENFILE
  #define NOSCROLL
  #define NOSERVICE
  #define NOSOUND
  #define NOTEXTMETRIC
  #define NOWH
  #define NOWINOFFSETS
  #define NOCOMM
  #define NOKANJI
  #define NOHELP
  #define NOPROFILER
  #define NODEFERWINDOWPOS
  #define NOMCX
#endif
#include <spdlog/spdlog.h>
#ifdef _WIN32
  #undef near
  #undef far
#endif

namespace engine::Core
{
  void InitLogging();
  std::shared_ptr<spdlog::logger> MakeUniversalLogger(const std::string& name);
}