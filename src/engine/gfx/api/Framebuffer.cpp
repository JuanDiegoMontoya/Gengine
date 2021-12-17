#include "../../PCH.h"
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
      0,
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

  void Framebuffer::SetAttachment(Attachment slot, const TextureView& view, uint32_t level)
  {
    glNamedFramebufferTexture(handle_, glAttachments[static_cast<uint32_t>(slot)], view.id_, level);
  }

  void Framebuffer::ResetAttachment(Attachment slot)
  {
    glNamedFramebufferTexture(handle_, glAttachments[static_cast<uint32_t>(slot)], 0, 0);
  }

  void Framebuffer::SetDrawBuffers(std::span<const Attachment> slots)
  {
    ASSERT(slots.size() < 8);
    GLenum buffers[8]{};
    for (size_t i = 0; i < slots.size(); i++)
    {
      ASSERT(slots[i] <= Attachment::COLOR_ATTACHMENT_MAX);
      buffers[i] = glAttachments[static_cast<uint32_t>(slots[i])];
    }
    glNamedFramebufferDrawBuffers(handle_, slots.size(), buffers);
  }

  bool Framebuffer::IsValid() const
  {
    GLenum status = glCheckNamedFramebufferStatus(handle_, GL_FRAMEBUFFER);
    return status == GL_FRAMEBUFFER_COMPLETE;
  }

  void Framebuffer::Bind()
  {
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

  void Framebuffer::Blit(const Framebuffer& source, const Framebuffer& target,
    Offset2D sourceStart, Offset2D sourceEnd,
    Offset2D targetStart, Offset2D targetEnd,
    AspectMaskBits mask, Filter filter)
  {
    glBlitNamedFramebuffer(source.handle_, target.handle_,
      sourceStart.x, sourceStart.y, sourceEnd.x, sourceEnd.y,
      targetStart.x, targetStart.y, targetEnd.x, targetEnd.y,
      getAspectMask(mask), glFilter[static_cast<uint32_t>(filter)]);
  }
}