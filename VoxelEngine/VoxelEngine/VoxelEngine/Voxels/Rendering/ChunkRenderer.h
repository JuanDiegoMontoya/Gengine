#pragma once
#include <Graphics/DynamicBuffer.h>
#include <Graphics/Shapes.h>

namespace ChunkRenderer
{
  void InitAllocator();
  void GenerateDrawCommandsGPU();
  void RenderNorm();

  // for debug
  void DrawBuffers();


  /* $$$$$$$$$$$$$$$   Culling pipeline stuff   $$$$$$$$$$$$$$$$

      phase 1:
        render normally
      phase 2:
        regenerate DIB (frustum + distance cull)
      phase 3:
        occlusion draw bounding boxes (BBs)
          any chunk whose BB was drawn will be set to draw in next frame
          no color or depth is written in this phase
      phase 4 (maybe optional):
        render chunks whose visibility changed to true
        will lower FPS, but will prevent temporal occlusion holes

  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

  void Render();          // phase 1
  void GenerateDIB();     // phase 2
  void RenderOcclusion(); // phase 3
  void RenderRest();      // phase 4

  void Update();

  inline std::unique_ptr<DynamicBuffer<AABB16>> allocator;

  struct Settings
  {
    // visibility
    float normalMin = 0;
    float normalMax = 800;
    bool freezeCulling = false;
    bool debug_drawOcclusionCulling = false;
  }inline settings;
}