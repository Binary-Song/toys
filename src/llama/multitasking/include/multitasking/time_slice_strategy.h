#pragma once
#include "common.h"
#include "foundation/foundation.h"
#include <vector>
#include <functional>
#include <coroutine>
#include <iostream>
#include <chrono>
#include "multitasking.h"
#include <spdlog/spdlog.h>
#include <memory>

namespace llama
{
namespace mt
{
template <typename Result>
class DeadlinePromise;
class DeadlineTaskContext;
class DeadlineScheduleAwaitable;
class DeadlineTaskAwaitable;

// 这是协程的返回类型。
template <typename Result>
class DeadlineTask
{
public:
	explicit DeadlineTask(std::coroutine_handle<DeadlinePromise<Result>> handle)
		: m_coro_handle(handle)
	{}

	std::coroutine_handle<DeadlinePromise<Result>> CoroutineHandle() const { return m_coro_handle; }

private:
	std::coroutine_handle<DeadlinePromise<Result>> m_coro_handle;
};

}//namespace mt
}//namespace llama

// 告诉编译器：返回 Task<DeadlineStrategy> 对应 DeadlinePromise
template <typename Result>
struct ::std::coroutine_traits<llama::mt::DeadlineTask<Result>, llama::mt::DeadlineTaskContext>
{
	using promise_type = llama::mt::DeadlinePromise<Result>;
};

namespace llama
{
namespace mt
{

/// 在写嵌套协程时，将context传入嵌套的协程。如：
///
/// ```
/// [](Context const& context) -> Task
/// {
///     ...
///     co_await [](Context const& context) -> Task {...} (context);
///     ...
/// }
/// ```
class DeadlineTaskContext
{
private:
	explicit DeadlineTaskContext(const TimePoint &deadline)
		: m_deadline(deadline)
	{}

public:
	TimePoint deadline() const { return m_deadline; }

private:
	TimePoint m_deadline;
};

template <typename Result>
class DeadlinePromise
{
	friend class DeadlineScheduleAwaitable;

public:
	virtual ~DeadlinePromise() = default;
	explicit DeadlinePromise(p<DeadlineTaskContext> context)
		: m_context(context)
	{}

	DeadlineTask<Result> get_return_object() noexcept;
	std::suspend_always initial_suspend() const noexcept { return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }

	void return_void() {}

	void return_value(Result &&ret) {}

	void return_value(Result const &ret) {}

	void unhandled_exception();
	std::suspend_always await_transform(std::suspend_always) const { return {}; }
	std::suspend_never await_transform(std::suspend_never) const { return {}; }
	DeadlineScheduleAwaitable await_transform(DeadlineSchedule);
	DeadlineTaskAwaitable await_transform(DeadlineTask<Result> const &task);

private:
	std::exception_ptr m_exception = nullptr;
	p<DeadlineTaskContext> m_context;
};


class DeadlineScheduleAwaitable
{
public:
	DeadlineScheduleAwaitable(p<DeadlinePromise<Result>> promise)
		: m_promise{promise}
	{}

	void await_suspend(DeadlineTask task) const noexcept {}
	void await_resume() const noexcept { task.resume(); }
	bool await_ready() const noexcept
	{
		return now() < m_promise->m_context.deadline();// false=挂起，true=忽略
	}

private:
	p<DeadlinePromise> m_promise;
};

class DeadlineTaskAwaitable
{
public:
	DeadlineTaskAwaitable(p<DeadlinePromise> promise)
		: m_promise{promise}
	{}

	void await_suspend(DeadlineTask task) const noexcept {}
	void await_resume() const noexcept {}
	bool await_ready() const noexcept
	{
		return now() < m_promise->m_context.deadline();// false=挂起，true=忽略
	}

private:
	p<DeadlinePromise> m_promise;
};

class DeadlineScheduler
{
public:
	using Schedule = DeadlineSchedule;

	DeadlineSchedule(Duration const &timeSliceDuration)
		: m_timeSliceDuration{timeSliceDuration}
	{}

	template <typename Func>
	void Submit(Func &&func)
	{
		Task<DeadlineStrategy> task = func();
		task.CoroutineHandle().promise().m_nextSusTime = now() + m_timeSliceDuration;
		m_tasks.emplace_back(true, task);
	}

	void Step();
	void Run();

private:
	Duration m_timeSliceDuration;
	std::vector<std::tuple<bool, DeadlineTask>> m_tasks;
};

}//namespace mt
}//namespace llama

//
//
//  implementations
//
//
//
namespace llama
{
namespace mt
{

template <typename Result>
DeadlineScheduleAwaitable DeadlinePromise<Result>::await_transform(DeadlineSchedule )
{
	return DeadlineScheduleAwaitable{this};
}

template <typename Result>
DeadlineTaskAwaitable DeadlinePromise<Result>::await_transform(DeadlineTask<Result> const &task)
{

}

}//namespace mt
}//namespace llama
