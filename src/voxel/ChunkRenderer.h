#pragma once
#include <engine/Shapes.h>
#include <memory>
#include <string>
#include <engine/gfx/Texture.h>

namespace GFX
{
  class TextureArray;
  class Texture2D;
}

namespace Voxels
{
  class ChunkRenderer
  {
  public:
    ChunkRenderer();
    ~ChunkRenderer();
    //void InitAllocator();

    // for debug
    void DrawBuffers();
    void Draw();

    uint64_t AllocChunk(std::span<int> vertices, std::span<uint32_t> indices, const AABB& aabb);
    void FreeChunk(uint64_t allocHandle);

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

  private:
    void RenderVisible();   // phase 1
    void GenerateDIB();     // phase 2
    void RenderOcclusion(); // phase 3
    void RenderRest();      // phase 4

    std::vector<glm::uvec2> GetAllocIndices();

    struct ChunkRendererStorage* data{};
  };
}