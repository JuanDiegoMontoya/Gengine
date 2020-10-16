#pragma once
#include <string>
#include <GAssert.h>

class Engine;
class Scene;

class Application
{
public:
  static void LoadScene(std::string_view name) { ASSERT(loadSceneStr); loadSceneStr(name); }
  static void LoadScene(unsigned index) { ASSERT(loadSceneIndex); loadSceneIndex(index); }
  static void UnloadScene(std::string_view name) { ASSERT(unloadSceneStr); unloadSceneStr(name); }
  static void UnloadScene(unsigned index) { ASSERT(unloadSceneIndex); unloadSceneIndex(index); }
  static bool IsPlaying() { return isPlaying_; }
  static void Start();
  static void Quit();

  static void SetStartCallback(void(*fn)(Scene*)) { start = fn; }
  static void SetLoadSceneCallback(void(*fn)(std::string_view)) { loadSceneStr = fn; }
  static void SetLoadSceneCallback(void(*fn)(unsigned)) { loadSceneIndex = fn; }
  static void SetUnloadSceneCallback(void(*fn)(std::string_view)) { unloadSceneStr = fn; }
  static void SetUnloadSceneCallback(void(*fn)(unsigned)) { unloadSceneIndex = fn; }

private:
  static inline void(*start)(Scene*) = nullptr;
  static inline void(*loadSceneStr)(std::string_view) = nullptr;
  static inline void(*loadSceneIndex)(unsigned) = nullptr;
  static inline void(*unloadSceneStr)(std::string_view) = nullptr;
  static inline void(*unloadSceneIndex)(unsigned) = nullptr;

  static inline bool isPlaying_ = false;

  static inline Engine* engine_ = nullptr;
};