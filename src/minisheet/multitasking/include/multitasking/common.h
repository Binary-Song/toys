#pragma once
#include "common.h"
#include <chrono>
#include <coroutine>

namespace minisheet
{
namespace mt
{

// ~~~~ 重要类型的前向声明 ~~~~

// 在协程中对它 co_await 表示在此处让Scheduler决定要不要挂起。
// 例：co_await Schedule<DeadlineStrategy>{};
class DeadlineSchedule;

// 调度器负责轮转执行协程，根据其策略决定协程在各个挂起点要不要挂起。
class DeadlineScheduler;

// 任务是调度器调度的对象。Result为协程的返回值。
template <typename Result>
class DeadlineTask;

// ~~~~ 方便工具 ~~~~

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using Duration = TimePoint::duration;

inline TimePoint now() noexcept
{
	return std::chrono::high_resolution_clock::now();
}

}//namespace mt

using Schedule = mt::DeadlineSchedule;
using Scheduler = mt::DeadlineScheduler;
template <typename Result>
using Task = mt::DeadlineTask<Result>;

}//namespace minisheet
