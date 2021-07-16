#pragma once

class Timer
{
public:
  Timer();
  void Reset();
  double Elapsed() const;

  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

private:
  alignas(8) char mem_[8];
};