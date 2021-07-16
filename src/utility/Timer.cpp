#include "Timer.h"
#include <chrono>
#include <new>

using second_ = std::chrono::duration<double, std::ratio<1> >;
using clock_ = std::chrono::high_resolution_clock;
using timepoint_ = std::chrono::time_point<clock_>;

Timer::Timer()
{
  static_assert(sizeof(timepoint_) == sizeof(mem_));
  new(mem_) timepoint_(clock_::now());
}

void Timer::Reset()
{
  new(mem_) timepoint_(clock_::now());
}

double Timer::Elapsed() const
{
  // beg to your god
  timepoint_ beg_ = *std::launder(reinterpret_cast<const timepoint_*>(mem_));
  return std::chrono::duration_cast<second_>(clock_::now() - beg_).count();
}
