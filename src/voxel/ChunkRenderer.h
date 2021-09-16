#pragma once
#include <cstdint>
#include <span>

namespace GFX
{
  class TextureArray;
  class Texture2D;
  struct RenderView;
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
    void DrawBuffers(std::span<GFX::RenderView> renderViews);
    void Draw(std::span<GFX::RenderView> renderViews);

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
    void RenderVisibleChunks(std::span<GFX::RenderView> renderViews);   // phase 1
    void GenerateDrawIndirectBuffer(std::span<GFX::RenderView> renderViews);     // phase 2
    void RenderOcclusion(std::span<GFX::RenderView> renderViews); // phase 3
    void RenderDisoccludedThisFrame(std::span<GFX::RenderView> renderViews);      // phase 4

    // PIMPL
    struct ChunkRendererStorage* data{};
  };
}