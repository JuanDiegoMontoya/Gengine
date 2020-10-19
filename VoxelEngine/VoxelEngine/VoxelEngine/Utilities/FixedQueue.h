#pragma once
#include <queue>
#include <deque>

// https://stackoverflow.com/questions/56334492/c-create-fixed-size-queue
template <typename T, int MaxLen, typename Container = std::deque<T>>
class FixedQueueB : public std::queue<T, Container>
{
public:
  void push(const T& value)
  {
    if (this->size() == MaxLen)
      this->c.pop_front();
    std::queue<T, Container>::push(value);
  }
};

// jank
template <typename T, int MaxLen>
class FixedQueue : public std::deque<T>
{
public:
  void push_front_fixed(const T& val)
  {
    if (this->size() == MaxLen)
      this->pop_front();
    this->push_front(val);
  }
};