#pragma once
#include <cstdint>
#include <optional>
#include "BasicTypes.h"

namespace GFX
{
  class TextureView;

  class Framebuffer
  {
  public:
    Framebuffer();
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& old) noexcept;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer& operator=(Framebuffer&&) = delete;
    ~Framebuffer();

    void SetAttachment(Attachment slot, TextureView& view, uint32_t level);
    void Bind();

    [[nodiscard]] bool IsValid() const;
    [[nodiscard]] uint32_t GetAPIHandle() const { return handle_; }
    [[nodiscard]] uint32_t GetAttachmentAPIHandle(Attachment slot) const;

    static void Blit(const Framebuffer& source, Framebuffer& destination,
      Offset2D sourceStart, Offset2D sourceEnd,
      Offset2D destinationStart, Offset2D destinationEnd,
      AspectMaskBits mask, Filter filter);

  private:
    uint32_t handle_{};
  };
}