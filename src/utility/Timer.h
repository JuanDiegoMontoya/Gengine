#pragma once

class Timer
{
public:
  Timer();
  void Reset();
  double Elapsed() const;
  double Elapsed_ms() const;

  Timer(const Timer& other);
  Timer& operator=(const Timer& other);

private:
  alignas(8) char mem_[8];
};

namespace ProgramTimer
{
  double TimeSeconds();
};