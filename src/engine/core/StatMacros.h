#pragma once
#include <engine/gfx/api/Fence.h>
#include <engine/core/Statistics.h>
#include <utility/Timer.h>
#include <utility/Defer.h>

#define SM_CONCAT(a, b) SM_CONCAT_INNER(a, b)
#define SM_CONCAT_INNER(a, b) a ## b

#define DECLARE_FLOAT_STAT(statID, statGroup) \
  static void* SM_CONCAT(sm_float_stat_helper, __LINE__) = []() \
  { \
    engine::Core::StatisticsManager::Get()->RegisterFloatStat(#statID, #statGroup); \
    return nullptr; \
  }();

#define MEASURE_GPU_TIMER_STAT(name) \
  static GFX::TimerQueryAsync SM_CONCAT(sm_timer, __LINE__) (5); \
  GFX::TimerScoped SM_CONCAT(sm_scopedTimer, __LINE__)(SM_CONCAT(sm_timer, __LINE__)); \
  do \
  { \
      auto SM_CONCAT(sm_result, __LINE__) = SM_CONCAT(sm_timer, __LINE__).Elapsed_ns(); \
      if (SM_CONCAT(sm_result, __LINE__)) \
      { \
        double SM_CONCAT(sm_time, __LINE__) = (double)*SM_CONCAT(sm_result, __LINE__) / 1000000.0; \
        engine::Core::StatisticsManager::Get()->PushFloatStatValue(#name, SM_CONCAT(sm_time, __LINE__)); \
        /*printf(#name ": %f\n", SM_CONCAT(sm_time, __LINE__));*/ \
      } \
  } while(0)

#define MEASURE_GPU_TIMER_STAT_SYNCHRONOUS(name) \
  GFX::TimerQuery SM_CONCAT(sm_timer, __LINE__); \
  Defer SM_CONCAT(sm_defer, __LINE__)([&SM_CONCAT(sm_timer, __LINE__)]() \
  { \
    double SM_CONCAT(sm_time, __LINE__) = (double)SM_CONCAT(sm_timer, __LINE__).Elapsed_ns() / 1000000.0; \
    engine::Core::StatisticsManager::Get()->PushFloatStatValue(#name, SM_CONCAT(sm_time, __LINE__)); \
    /*printf(#name ": %f\n", SM_CONCAT(sm_time, __LINE__));*/ \
  })

#define MEASURE_CPU_TIMER_STAT(name) \
  static Timer SM_CONCAT(sm_timer, __LINE__); \
  SM_CONCAT(sm_timer, __LINE__).Reset(); \
  Defer SM_CONCAT(sm_defer, __LINE__)([]() \
  { \
    engine::Core::StatisticsManager::Get()->PushFloatStatValue(#name, SM_CONCAT(sm_timer, __LINE__).Elapsed_ms()); \
  })