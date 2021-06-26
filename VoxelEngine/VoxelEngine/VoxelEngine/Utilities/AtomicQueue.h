#pragma once
#include <queue>
#include <shared_mutex>

template<typename T>
class AtomicQueue
{
public:
  AtomicQueue() {};

  void Push(const T& val)
  {
    std::lock_guard lck(mutex_);
    queue_.push(val);
  }

  T Pop()
  {
    std::lock_guard lck(mutex_);
    T val = queue_.front();
    queue_.pop();
    return val;
  }

  // single lock, low-overhead
  template<typename Callback>
  void ForEach(Callback fn, unsigned maxIterations = UINT32_MAX)
  {
    if (maxIterations == 0)
    {
      maxIterations = UINT32_MAX;
    }

    std::lock_guard lck(mutex_);
    while (!queue_.empty() && maxIterations != 0)
    {
      T val = queue_.front();
      queue_.pop();
      fn(val);
      maxIterations--;
    }
  }

private:
  std::queue<T> queue_;
  mutable std::shared_mutex mutex_;
};