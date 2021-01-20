#pragma once
#include <cstdint>

typedef struct __GLsync* GLsync;

/*
  A non-blocking fence sync object.

  Usage:
  Create the Fence object *after* commands that rely on some protected memory
  (e.g. commands that cause the GPU to read or write to memory).

  Next, call Sync() on the previously made Fence immediately *before* modifying 
  protected memory. All commands issued before the fence object was created must 
  complete before Sync() will return.

  Notes:
  The timeout is infinite(!), so ensure you are properly double/triple buffering
  if you do not want Sync() to block. In those cases, treat this class as a 
  sanity check.

  Use this class primarily for ensuring persistently mapped buffers do not write 
  while the GPU is using them.
*/
namespace GFX
{
  class Fence
  {
  public:
    Fence();
    ~Fence();
    Fence(const Fence&) = delete;
    Fence& operator=(const Fence&) = delete;
    Fence(Fence&&) noexcept = delete;
    Fence& operator=(Fence&&) noexcept = delete;

    // returns how long (in ns) we were blocked for
    uint64_t Sync();

  private:
    GLsync sync_;
  };

  class TimerQuery
  {
  public:
    TimerQuery();
    ~TimerQuery();
    TimerQuery(const TimerQuery&) = delete;
    TimerQuery& operator=(const TimerQuery&) = delete;
    TimerQuery(TimerQuery&&) noexcept = delete;
    TimerQuery& operator=(TimerQuery&&) noexcept = delete;

    // returns how (in ns) we blocked for (invalidates this object) (BLOCKS)
    uint64_t Elapsed();

  private:
    GLuint queries[2];
  };
}