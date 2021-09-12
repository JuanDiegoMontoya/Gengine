#pragma once
#include <engine/GAssert.h>
#include <engine/Timestep.h>

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
  static void SetUpdateCallback(void(*fn)(Timestep)) { update = fn; }
  static void SetDrawOpaqueCallback(void(*fn)(Scene*, Timestep)) { drawOpaque = fn; }
  static void SetLoadSceneCallback(void(*fn)(const char*)) { loadSceneStr = fn; }
  static void SetLoadSceneCallback(void(*fn)(unsigned)) { loadSceneIndex = fn; }
  static void SetUnloadSceneCallback(void(*fn)(const char*)) { unloadSceneStr = fn; }
  static void SetUnloadSceneCallback(void(*fn)(unsigned)) { unloadSceneIndex = fn; }

private:
  static inline void(*start)(Scene*) = nullptr;
  static inline void(*update)(Timestep) = nullptr;
  static inline void(*drawOpaque)(Scene*, Timestep) = nullptr;
  static inline void(*loadSceneStr)(const char*) = nullptr;
  static inline void(*loadSceneIndex)(unsigned) = nullptr;
  static inline void(*unloadSceneStr)(const char*) = nullptr;
  static inline void(*unloadSceneIndex)(unsigned) = nullptr;

  static inline bool isPlaying_ = false;

  static inline Engine* engine_ = nullptr;
};