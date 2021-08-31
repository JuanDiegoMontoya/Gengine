#pragma once
#include <concepts>
#include <array>
#include <utility/HashedString.h>

namespace engine::Core
{
  template<typename T>
  concept Numeric =
    std::integral<T> ||
    std::floating_point<T>;

  // records statistics about the most recent N elements pushed to it
  template<Numeric T, size_t N>
  class StatBuffer
  {
  public:
    void Push(T val)
    {
      auto prev = std::exchange(buffer_[next_], val);
      next_ = (next_ + 1) % N;

      if (count_ < N)
        count_++;

      sum_ += val - prev;
    }

    double Sum() const
    {
      //double sum = 0;
      //for (size_t i = 0; i < count_; i++)
      //  sum += buffer_[i];
      //return sum;
      return sum_;
    }

    double Mean() const
    {
      return Sum() / count_;
    }

    double Variance() const
    {
      const auto mean = Mean();
      double numerator = 0;
      for (size_t i = 0; i < count_; i++)
        numerator += (buffer_[i] - mean) * (buffer_[i] - mean);
      return numerator / count_;
    }

    double StandardDeviation() const
    {
      return sqrt(Variance());
    }

    T Min() const
    {
      auto min = buffer_[0];
      for (size_t i = 1; i < count_; i++)
        if (buffer_[i] < min) min = buffer_[i];
      return min;
    }

    T Max() const
    {
      auto max = buffer_[0];
      for (size_t i = 1; i < count_; i++)
        if (buffer_[i] > max) max = buffer_[i];
      return max;
    }

  private:
    std::array<T, N> buffer_{};
    size_t next_{};
    size_t count_{};
    double sum_{};
  };

  constexpr size_t STATISTICS_BUFFER_SIZE = 500;

  class StatisticsManager
  {
  public:
    static StatisticsManager* Get();

    void RegisterFloatStat(hashed_string statName, hashed_string groupName);
    void PushFloatStatValue(hashed_string statName, float val);
    const StatBuffer<float, STATISTICS_BUFFER_SIZE>& GetFloatStat(hashed_string statName);

    // solemnly promise that you'll only call this in one place at most
    void DrawUI();

    StatisticsManager(const StatisticsManager&) = delete;
    StatisticsManager(StatisticsManager&&) = delete;
    StatisticsManager& operator=(const StatisticsManager&) = delete;
    StatisticsManager& operator=(StatisticsManager&&) = delete;

  private:
    StatisticsManager();
    ~StatisticsManager();

    struct StatisticsManagerData* data{};
  };
}