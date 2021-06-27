#pragma once
#include <CoreEngine/GAssert.h>

class Engine;
class Scene;

class Application
{
public:
  static void LoadScene(const char* name) { ASSERT(loadSceneStr); loadSceneStr(name); }
  static void LoadScene(unsigned index) { ASSERT(loadSceneIndex); loadSceneIndex(index); }
  static void UnloadScene(const char* name) { ASSERT(unloadSceneStr); unloadSceneStr(name); }
  static void UnloadScene(unsigned index) { ASSERT(unloadSceneIndex); unloadSceneIndex(index); }
  static bool IsPlaying() { return isPlaying_; }
  static void Start();
  static void Shutdown();
  static void Quit();

  static void SetStartCallback(void(*fn)(Scene*)) { start = fn; }
  static void SetUpdateCallback(void(*fn)(float)) { update = fn; }
  static void SetDrawCallback(void(*fn)(float)) { draw = fn; }
  static void SetLoadSceneCallback(void(*fn)(const char*)) { loadSceneStr = fn; }
  static void SetLoadSceneCallback(void(*fn)(unsigned)) { loadSceneIndex = fn; }
  static void SetUnloadSceneCallback(void(*fn)(const char*)) { unloadSceneStr = fn; }
  static void SetUnloadSceneCallback(void(*fn)(unsigned)) { unloadSceneIndex = fn; }

private:
  static inline void(*start)(Scene*) = nullptr;
  static inline void(*update)(float) = nullptr;
  static inline void(*draw)(float) = nullptr;
  static inline void(*loadSceneStr)(const char*) = nullptr;
  static inline void(*loadSceneIndex)(unsigned) = nullptr;
  static inline void(*unloadSceneStr)(const char*) = nullptr;
  static inline void(*unloadSceneIndex)(unsigned) = nullptr;

  static inline bool isPlaying_ = false;

  static inline Engine* engine_ = nullptr;
};