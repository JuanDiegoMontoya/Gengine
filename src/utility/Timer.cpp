#include "Timer.h"
#include <chrono>
#include <new>

using second_t = std::chrono::duration<double, std::ratio<1> >;
using myclock_t = std::chrono::high_resolution_clock;
using timepoint_t = std::chrono::time_point<myclock_t>;

Timer::Timer()
{
  static_assert(sizeof(timepoint_t) == sizeof(mem_));
  new(mem_) timepoint_t(myclock_t::now());
}

void Timer::Reset()
{
  new(mem_) timepoint_t(myclock_t::now());
}

double Timer::Elapsed() const
{
  // beg to your god
  timepoint_t beg_ = *std::launder(reinterpret_cast<const timepoint_t*>(mem_));
  return std::chrono::duration_cast<second_t>(myclock_t::now() - beg_).count();
}
