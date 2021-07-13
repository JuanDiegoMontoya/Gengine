#pragma once
#include <concepts>
#include <utility>

template<std::invocable Fn>
class Defer
{
public:
  constexpr Defer(Fn&& f) noexcept : f_(std::move(f)) {}
  constexpr Defer(const Fn& f) : f_(f) {}
  constexpr ~Defer() { if (!dismissed_) f_(); }

  constexpr void Cancel() noexcept { dismissed_ = true; }

  Defer(const Defer&) = delete;
  Defer(Defer&&) = delete;
  Defer& operator=(const Defer&) = delete;
  Defer& operator=(Defer&&) = delete;

private:
  bool dismissed_{ false };
  Fn f_;
};