#pragma once
#include <cstdint>
#include <span>

namespace GFX
{
  class TextureArray;
  class Texture2D;
}

struct AABB;

namespace Voxels
{
  class ChunkRenderer
  {
  public:
    ChunkRenderer();
    ~ChunkRenderer();

    // for debug
    void DrawBuffers();
    void Draw();

    uint64_t AllocChunkMesh(std::span<uint32_t> vertices, const AABB& aabb);
    void FreeChunkMesh(uint64_t allocHandle);

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
          render chunks whose visibility changed to true this frame
          will lower FPS, but will prevent temporal occlusion holes

    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

  private:
    void RenderVisible();   // phase 1
    void GenerateDIB();     // phase 2
    void RenderOcclusion(); // phase 3
    void RenderRest();      // phase 4

    // PIMPL
    struct ChunkRendererStorage* data{};
  };
}