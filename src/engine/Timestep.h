#pragma once

struct Timestep
{
  double dt_actual{};   // Not affected by timescale. Use for effects that take place in real time.
  double dt_effective{};// Affected by timescale. Use for gameplay systems by default.
};