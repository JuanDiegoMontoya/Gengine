#include "Timer.h"
#include <chrono>
#include <new>

using second_t = std::chrono::duration<double, std::ratio<1>>;
using millisecond_t = std::chrono::duration<double, std::milli>;
using microsecond_t = std::chrono::duration<double, std::micro>;
using nanosecond_t = std::chrono::nanoseconds;
using myclock_t = std::chrono::high_resolution_clock;
using timepoint_t = std::chrono::time_point<myclock_t>;

Timer::Timer()
{
  static_assert(sizeof(timepoint_t) == sizeof(mem_), "You need to change the size of the internal storage!");
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

double Timer::Elapsed_ms() const
{
  timepoint_t beg_ = *std::launder(reinterpret_cast<const timepoint_t*>(mem_));
  return std::chrono::duration_cast<millisecond_t>(myclock_t::now() - beg_).count();
}

Timer::Timer(const Timer& other)
{
  *this = other;
}

Timer& Timer::operator=(const Timer& other)
{
  if (&other == this) return *this;
  timepoint_t beg_ = *std::launder(reinterpret_cast<const timepoint_t*>(other.mem_));
  new(mem_) timepoint_t(beg_);
  return *this;
}

static timepoint_t programBegin = myclock_t::now();
double ProgramTimer::TimeSeconds()
{
  return std::chrono::duration_cast<second_t>(myclock_t::now() - programBegin).count();
}
