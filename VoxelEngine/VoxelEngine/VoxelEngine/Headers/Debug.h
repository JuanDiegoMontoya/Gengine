#pragma once
#include <entt.hpp>

namespace Debug 
{
	//extern std::map<std::string, double> systemsPerfInfo;

	void Update(float dt);
	void PushPerfInfo(std::pair<entt::hashed_string, double>&&);
}

#if DE_BUG
#ifdef TRACY_ENABLE
#define PERF_BENCHMARK_START ZoneScoped
#define PERF_BENCHMARK_END sizeof(int)
#else
#define PERF_BENCHMARK_START  high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now(); 
#define PERF_BENCHMARK_END \
	duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_); \
	std::pair<entt::hashed_string, double> benchmark_data_; \
	benchmark_data_.first = __FUNCTION__; \
	benchmark_data_.second = benchmark_duration_.count() * 1000; \
	Debug::PushPerfInfo(benchmark_data_);
#endif // TRACY_ENABLE

#define DEBUG_DO(x) x
#else
//noops
#define PERF_BENCHMARK_START do { (void)sizeof(int); } while(0)
#define PERF_BENCHMARK_END do { (void)sizeof(int); } while(0)
#define DEBUG_DO(x) do { (void)sizeof(int); } while(0) //noop
#endif