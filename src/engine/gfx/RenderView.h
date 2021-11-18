#pragma once
#include <utility/Flags.h>
#include <cstdint>
#include "BasicTypes.h"

namespace GFX
{
  class Framebuffer;
  struct Camera;

  // this enum specifies a bit mask to determine what should be rendered into the view
  // this is inevitably going to become a hacky mess, better here than elsewhere!
  enum class RenderMaskBit : uint32_t
  {
    None                  = 0,
    
    // render voxels
    RenderVoxels          = 1 << 0,

    // render ordinary objects
    RenderObjects         = 1 << 1,

    // render particle emitters
    RenderEmitters        = 1 << 2,

    // render "screen elements", essentially non-ImGui UI stuff that may exist in world space
    RenderScreenElements  = 1 << 3,

    // if RenderVoxels is true, render only chunks near the camera
    // the distance is determined by a quality setting
    // this should only be used for probes or other low-quality rendering purposes
    RenderVoxelsNear      = 1 << 4,

    // render the skybox
    RenderSky             = 1 << 5,

    // render atmospheric fog
    RenderFog             = 1 << 6,

    // clear color at the start of each frame
    ClearColorEachFrame   = 1 << 7,

    // clear color at the start of each frame
    ClearDepthEachFrame   = 1 << 8,

    // early fog pass (before shading) for probes
    RenderEarlyFog        = 1 << 9,
  };
  DECLARE_FLAG_TYPE(RenderMask, RenderMaskBit, uint32_t)

  struct RenderView
  {
    GFX::Framebuffer* renderTarget{};
    GFX::Camera* camera{};
    RenderMask mask{};
    Offset2D offset{};
    Extent2D size{};

    bool operator==(const RenderView&) const = default;
  };

  template<typename T> struct hash;

  template<>
  struct hash<RenderView>
  {
    std::size_t operator()(const RenderView& a) const noexcept;
  };
}