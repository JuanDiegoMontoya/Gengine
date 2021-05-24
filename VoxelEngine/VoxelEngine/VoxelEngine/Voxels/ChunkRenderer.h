#pragma once
#include <CoreEngine/DynamicBuffer.h>
#include <CoreEngine/Shapes.h>

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


    // TODO: make private after NuRenderer no longer needs this
    struct Settings
    {
      // visibility
      float normalMin = 0;
      float normalMax = 2000;
      bool freezeCulling = false;
      bool debug_drawOcclusionCulling = false;
    }settings;

  private:
    friend class ChunkMesh;
    friend class VoxelManager;

    void RenderVisible();   // phase 1
    void GenerateDIB();     // phase 2
    void RenderOcclusion(); // phase 3
    void RenderRest();      // phase 4
    void Update();

    std::unique_ptr<GFX::DynamicBuffer<AABB16>> allocator;
    std::unique_ptr<GFX::VAO> vao;
    std::unique_ptr<GFX::StaticBuffer> dib;

    std::unique_ptr<GFX::StaticBuffer> drawCountGPU;

    // size of compute block  for the compute shader
    const int blockSize = 64; // defined in compact_batch.cs

    // resets each frame BEFORE the culling phase
    //GLuint allocDataBuffer = 0;
    std::unique_ptr<GFX::VAO> vaoCull;
    //std::unique_ptr<GFX::StaticBuffer> vboCull; // stores only cube vertices
    std::unique_ptr<GFX::StaticBuffer> dibCull;
    GLsizei activeAllocs;
    std::pair<uint64_t, GLuint> stateInfo{ 0, 0 };
    bool dirtyAlloc = true;
    std::unique_ptr<GFX::StaticBuffer> allocBuffer;

    // resources
    std::unique_ptr<GFX::TextureArray> textures;
    std::unique_ptr<GFX::Texture2D> blueNoise64;
  };
}