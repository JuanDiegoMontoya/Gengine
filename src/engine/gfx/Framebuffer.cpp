#include "../PCH.h"
#include "Framebuffer.h"
#include <utility>
#include <glad/glad.h>
#include "Texture.h"

namespace GFX
{
  namespace // detail
  {
    GLenum glAttachments[] =
    {
      GL_NONE,
      GL_COLOR_ATTACHMENT0,
      GL_COLOR_ATTACHMENT1,
      GL_COLOR_ATTACHMENT2,
      GL_COLOR_ATTACHMENT3,

      GL_DEPTH_ATTACHMENT,
      GL_STENCIL_ATTACHMENT,
      GL_DEPTH_STENCIL_ATTACHMENT
    };

    GLenum glFilter[] =
    {
      GL_NEAREST,
      GL_LINEAR,
    };

    GLbitfield getAspectMask(AspectMaskBits bits)
    {
      GLbitfield ret = 0;
      ret |= bits & AspectMaskBit::COLOR_BUFFER_BIT ? GL_COLOR_BUFFER_BIT : 0;
      ret |= bits & AspectMaskBit::DEPTH_BUFFER_BIT ? GL_DEPTH_BUFFER_BIT : 0;
      ret |= bits & AspectMaskBit::STENCIL_BUFFER_BIT ? GL_STENCIL_BUFFER_BIT : 0;
      return ret;
    }
  }

  Framebuffer::Framebuffer()
  {
    glCreateFramebuffers(1, &handle_);
  }

  std::optional<Framebuffer> Framebuffer::Create()
  {
    return Framebuffer();
  }

  Framebuffer::Framebuffer(Framebuffer&& old) noexcept
  {
    handle_ = std::exchange(old.handle_, 0);
  }

  Framebuffer& Framebuffer::operator=(Framebuffer&& old) noexcept
  {
    if (this == &old) return *this;
    this->~Framebuffer();
    handle_ = std::exchange(old.handle_, 0);
    return *this;
  }

  Framebuffer::~Framebuffer()
  {
    if (handle_ != 0)
    {
      glDeleteFramebuffers(1, &handle_);
    }
  }

  void Framebuffer::SetAttachment(Attachment slot, TextureView& view, uint32_t level)
  {
    glNamedFramebufferTexture(handle_, glAttachments[static_cast<uint32_t>(slot)], view.id_, level);
  }

  void Framebuffer::SetDrawBuffers(std::span<const Attachment> slots)
  {
    ASSERT(slots.size() < 8);
    GLenum buffers[8]{};
    for (uint32_t i = 0; i < slots.size(); i++)
    {
      buffers[i] = glAttachments[static_cast<uint32_t>(slots[i])];
    }
    glNamedFramebufferDrawBuffers(handle_, slots.size(), buffers);
  }

  bool Framebuffer::IsValid() const
  {
    return glCheckNamedFramebufferStatus(handle_, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
  }

  void Framebuffer::Bind()
  {
    ASSERT(IsValid());
    glBindFramebuffer(GL_FRAMEBUFFER, handle_);
  }

  uint32_t Framebuffer::GetAttachmentAPIHandle(Attachment slot) const
  {
    GLint params{};
    glGetNamedFramebufferAttachmentParameteriv(
      handle_,
      glAttachments[static_cast<uint32_t>(slot)],
      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
      &params);
    return static_cast<uint32_t>(params);
  }

  void Framebuffer::Blit(const Framebuffer& source, Framebuffer& destination,
    Offset2D sourceStart, Offset2D sourceEnd,
    Offset2D destinationStart, Offset2D destinationEnd,
    AspectMaskBits mask, Filter filter)
  {
    glBlitNamedFramebuffer(source.handle_, destination.handle_,
      sourceStart.x, sourceStart.y, sourceEnd.x, sourceEnd.y,
      destinationStart.x, destinationStart.y, destinationEnd.x, destinationEnd.y,
      getAspectMask(mask), glFilter[static_cast<uint32_t>(filter)]);
  }
}