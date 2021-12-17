#pragma once
#include "api/Texture.h"
#include "api/BasicTypes.h"
#include <array>
#include <optional>

namespace GFX
{
  struct ClearColorValue
  {
    float f[4];
  };

  struct ClearDepthStencilValue
  {
    float depth{};
    int32_t stencil{};
  };

  union ClearValue
  {
    ClearColorValue color;
    ClearDepthStencilValue depthStencil;
  };

  struct RenderAttachment
  {
    TextureView* textureView{ nullptr };
    bool clearEachFrame{ false };
    ClearValue clearValue;
  };

  // This structure describes the render targets that may be used in a draw.
  // Inspired by VkRenderingInfoKHR of Vulkan's dynamic rendering extension.
  struct RenderInfo
  {
    constexpr static size_t maxColorAttachments = 4;
    Offset2D offset{};
    Extent2D size{};
    std::array<std::optional<RenderAttachment>, maxColorAttachments> colorAttachments;
    std::optional<RenderAttachment> depthAttachment{ std::nullopt };
    std::optional<RenderAttachment> stencilAttachment{ std::nullopt };
  };

  void SetViewport(const RenderInfo& renderInfo);
}