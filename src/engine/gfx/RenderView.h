#pragma once
#include <utility/Flags.h>
#include <cstdint>

namespace GFX
{
  class Framebuffer;
  struct Camera;

  // this enum specifies a bit mask to determine what should be rendered into the view
  // this is inevitably going to become a hacky mess, better here than elsewhere!
  enum class RenderMaskBit : uint32_t
  {
    MyCullableThing,
  };
  DECLARE_FLAG_TYPE(RenderMask, RenderMaskBit, uint32_t)

  struct RenderView
  {
    GFX::Framebuffer* renderTarget{};
    GFX::Camera* camera{};
    RenderMask mask{};
  };
}