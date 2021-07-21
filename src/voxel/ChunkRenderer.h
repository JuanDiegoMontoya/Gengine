#pragma once
#include <engine/gfx/DynamicBuffer.h>
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
    void InitAllocator();
    void GenerateDrawCommandsGPU();
    void RenderNorm();

    // for debug
    void DrawBuffers();
    void Draw();


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
    friend class ChunkMesh;
    friend class VoxelManager;

    void RenderVisible();   // phase 1
    void GenerateDIB();     // phase 2
    void RenderOcclusion(); // phase 3
    void RenderRest();      // phase 4
    void Update();

    std::unique_ptr<GFX::DynamicBuffer<AABB16>> verticesAllocator;
    std::unique_ptr<GFX::DynamicBuffer<>> indicesAllocator;
    GLuint vao{};
    std::unique_ptr<GFX::StaticBuffer> dib;

    std::unique_ptr<GFX::StaticBuffer> drawCountGPU;

    // size of compute block  for the compute shader
    const int groupSize = 64; // defined in compact_batch.cs

    GLuint vaoCull{};
    std::unique_ptr<GFX::StaticBuffer> dibCull;
    GLsizei activeAllocs{};
    std::pair<uint64_t, GLuint> stateInfo{ 0, 0 };
    bool dirtyAlloc = true;
    std::unique_ptr<GFX::StaticBuffer> vertexAllocBuffer;
    std::unique_ptr<GFX::StaticBuffer> indexAllocBuffer;

    // resources
    std::optional<GFX::Texture> blockTextures;
    std::optional<GFX::TextureView> blockTexturesView;
    std::optional<GFX::TextureSampler> blockTexturesSampler;

    std::optional<GFX::Texture> blueNoiseTexture;
    std::optional<GFX::TextureView> blueNoiseView;
    std::optional<GFX::TextureSampler> blueNoiseSampler;
  };
}