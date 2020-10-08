#pragma once
#include <Refactor/sun.h>
#include <memory>

class TextureArray;
class Sun;

namespace NuRenderer
{
  TextureArray* GetBlockTextures();

  void Init();
  void CompileShaders();
  void Clear();
  void DrawAll();

  void drawChunks();
  void splatChunks();
  void drawChunksWater();

  // generic drawing functions
  void drawAxisIndicators();
  void drawQuad();
  void DrawCube();

  inline int drawCalls = 0;
  inline std::unique_ptr<Sun> activeSun_;

  struct Settings
  {
    bool gammaCorrection = true;
    float fogStart = 500.f;
    float fogEnd = 3000.f;
  }inline settings;
}