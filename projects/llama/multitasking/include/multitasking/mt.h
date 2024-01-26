// todo: 1. 保护同task多次add (ok)
//       2. 考虑corotine_handle::done()的data race (ok)
//       3. 分享时间片? 这个要考虑如果用 await_suspend 的返回 coroutine_handle 的版本,如何判断被转移的
//       协程是不是已经在运行?? (closed)
//       4. 何时删除task? (重要)
// 如果没人拿走result/exception,那是不是不要删除了?
// bug:只创建协程不co_await好像会泄露
//       5. 实现scheduler,要求协程在co_await 另一个协程时,不能被唤醒.要等后者完成了才能唤醒.

// 结论
// 不能提供 add Task 的接口,因为会有傻逼多次add同一个task.
// 只能用 Context参数,实现*协程的启动时自动submit*.这样即使调用同一个协程2次也是分开的2个Task
// 或者把 scheduler 作为参数
// 既然自动submit,就实现不了分享时间片了.因为根本不知道被转移的协程是不是在运行.


#include <iostream>
#include <coroutine>
#include <chrono>
#include <list>
#include <mutex>
#include <memory>

/*  _____________________________  */
/*             声 明               */
/*  _____________________________  */

namespace mt
{
	template<typename T>
	using p = T*;

	template<typename T>
	using np = T*;

	using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
	using Duration = std::chrono::duration<long long>;

	inline TimePoint Now()
	{
		return std::chrono::high_resolution_clock::now();
	}

#define MT_NOTHING
#define MT_FORWARD_DECLARE_CLASSES(prefix) \
	prefix class BasicPromise; \
	prefix class ScheduleAwaitable; \
	template<typename _OuterTaskResult> \
	prefix class Promise; \
	template<typename _OuterTaskResult> \
	prefix class Task; \
	prefix class Scheduler; \
	template<typename _OuterTaskResult, typename _InnerTaskResult> \
	prefix class TaskAwaitable

	MT_FORWARD_DECLARE_CLASSES(MT_NOTHING);

	class Schedule {};
	using BasicCoroutineHandle = std::coroutine_handle<BasicPromise>;

}

template<typename OuterTaskResult>
struct std::coroutine_traits<mt::Task<OuterTaskResult>, mt::p<mt::Scheduler>>
{
	using promise_type = mt::Promise<OuterTaskResult>;
};

namespace mt
{
	// 表示协程的三种状态
	enum class PromiseState
	{
		// 协程仍未执行到 final_suspend
		NotDone,
		// 协程结束,抛出了一个异常
		HasException,
		// 协程结束,并成功给出了返回值
		HasResult,
	};

	class BasicPromise : public std::enable_shared_from_this<BasicPromise>
	{
		MT_FORWARD_DECLARE_CLASSES(friend);

	public:

		std::suspend_always final_suspend() noexcept
		{
			try
			{
				std::lock_guard<std::mutex> lock{ m_result_mtx };
				m_finished = true;
				return {};
			}
			catch (...)
			{
				return {};
			}
		}

	protected:
		explicit BasicPromise(p<Scheduler> scheduler);

		void Resume()
		{
			BasicCoroutineHandle::from_promise(*this).resume();
		}

		// 返回协程是否已结束.协程可能
		// (1)以成功结束,这时可以取到协程的返回值.
		// (2)以抛出异常结束,这时可以取到协程的异常对象.
		bool Done() const
		{
			std::lock_guard<std::mutex> lock(m_result_mtx);
			// 不用 std:;coroutine_handle::done 确保线程安全
			return m_finished;
		}

		PromiseState State() const
		{
			std::lock_guard<std::mutex> lock(m_result_mtx);
			if (m_finished)
			{
				if (m_exception == nullptr)
					return PromiseState::HasResult;
				else
					return PromiseState::HasException;
			}
			else
			{
				return PromiseState::NotDone;
			}
		}

	protected:
		// p.s.没有scheduler成员,因为没必要.设置flag给scheduler看即可.
		// 直接调用scheduler的方法僭越了.
		
		// 我是否在等别的任务
		bool               m_waiting = false;
		// 当前时间片的截止时间
		TimePoint          m_deadline = {};
		// 谁在等我,空表示没有任务在等我
		np<BasicPromise>   m_awaiter = {};

		// 这个mutex管它下面的几个成员, 以及m_result
		mutable std::mutex m_result_mtx = {};
		bool               m_finished = false;
		std::exception_ptr m_exception = {};
	};


	class ScheduleAwaitable
	{
		MT_FORWARD_DECLARE_CLASSES(friend);

		explicit ScheduleAwaitable(p<BasicPromise> promise)
			: m_promise{ promise }
		{}

	public:
		bool await_ready()
		{
			return Now() < m_promise->m_deadline;
		}

		void await_suspend() {}

		// 一旦resume,被调协程就被destory了.但返回值/异常对象会被移动出去/重抛出去.
		void await_resume() {}

	private:
		p<BasicPromise> m_promise;
	};

	template<typename OuterTaskResult, typename InnerTaskResult>
	class TaskAwaitable
	{
		MT_FORWARD_DECLARE_CLASSES(friend);

		explicit TaskAwaitable(p<Promise<OuterTaskResult>> awaiter, p<Promise<InnerTaskResult>> awaitee)
			: m_awaiter{ awaiter }, m_awaitee{ awaitee }
		{}

	public:
		bool await_ready()
		{
			return m_awaitee->Done();
		}

		void await_suspend()
		{
			m_awaitee->m_awaiter = m_awaiter;
			m_awaiter->m_waiting = true;
		}

		InnerTaskResult await_resume();

	private:
		p<Promise<OuterTaskResult>> m_awaiter;
		p<Promise<InnerTaskResult>> m_awaitee;
	};

	template<typename OuterTaskResult>
	class TaskAwaitable<OuterTaskResult, void>
	{
		MT_FORWARD_DECLARE_CLASSES(friend);

	public:
		explicit TaskAwaitable(p<Promise<void>> promise)
			: m_promise{ promise }
		{}

		bool await_ready()
		{
			return m_promise->Done();
		}

		void await_suspend()
		{
			m_awaitee->m_awaiter = m_awaiter;
			m_awaiter->m_waiting = true;
		}

		void await_resume();

	private:
		p<Promise<void>> m_promise;
	};

	// Task<Result> 对应的承诺类型。更新这里记得更新 Promise<void>
	template<typename Result>
	class Promise : public BasicPromise
	{
		MT_FORWARD_DECLARE_CLASSES(friend);

	public:
		explicit Promise(p<Scheduler> scheduler);

		Task<Result> get_return_object();

		std::suspend_always initial_suspend() const { return {}; }

		void unhandled_exception()
		{
			std::lock_guard<std::mutex> lock(m_result_mtx);
			m_exception = std::current_exception();
		}

		void return_value(Result&& result)
		{
			std::lock_guard<std::mutex> lock(m_result_mtx);
			m_result = std::move(result);
		}

		void return_value(Result const& result)
		{
			return return_value(Result{ result });
		}

		void return_value(Result& result)
		{
			return return_value(std::move(result));
		}

		// co_await Schedule{}
		ScheduleAwaitable await_transform(Schedule const& tag)
		{
			return ScheduleAwaitable{};
		}

		// co_await SomeNestedCoroutine(...);
		template<typename InnerTaskResult>
		TaskAwaitable<Result, InnerTaskResult> await_transform(Task<InnerTaskResult> const& subtask);

	private:
		Result m_result;
	};

	template<>
	class Promise<void> : public BasicPromise
	{
		MT_FORWARD_DECLARE_CLASSES(friend);

	public:
		explicit Promise(p<Scheduler> scheduler);
		
		Task<void> get_return_object();
		
		std::suspend_always initial_suspend() const { return {}; }

		void return_void() {}

		// co_await Schedule{}
		ScheduleAwaitable await_transform(Schedule const& tag)
		{
			return ScheduleAwaitable{ this };
		}

		// co_await SomeNestedCoroutine(...);
		template<typename InnerTaskResult>
		TaskAwaitable<void, InnerTaskResult> await_transform(Task<InnerTaskResult> const& task);
	};

	template<typename OuterTaskResult>
	class Task
	{
		MT_FORWARD_DECLARE_CLASSES(friend);

	private:
		static void DestroyPromise(Promise<OuterTaskResult>* promise)
		{
			auto handle = std::coroutine_handle<Promise<OuterTaskResult>>::from_promise(*promise);
			handle.destroy();
		}

		explicit Task(std::coroutine_handle<Promise<OuterTaskResult>> handle)
			: m_promise{ &handle.promise(), &Task::DestroyPromise }
		{}

		std::shared_ptr<Promise<OuterTaskResult>> m_promise;
	};

	class Scheduler
	{
		MT_FORWARD_DECLARE_CLASSES(friend);

		template<typename OuterTaskResult>
		friend class Promise;

	public:
		explicit Scheduler(Duration ration) : m_ration{ ration } {}

		void Run()
		{
			while (true)
			{
				{
					std::lock_guard<std::mutex> lock{ m_add_list_mtx };
					// 将 add_list 里的所有内容移动到 active_list 尾部
					m_active_list.splice(m_active_list.end(), m_add_list);
				}
				auto iter = m_active_list.begin();
				while (iter != m_active_list.end())
				{
					auto promise = *iter;
					promise->m_deadline = Now() + m_ration;
					promise->Resume();

					enum
					{
						Erase,
						MoveToWaitList,
						Next,
					} decision = {};

					if (promise->m_waiting)
					{
						decision = MoveToWaitList;
					}
					else
					{
						if (promise->Done())
						{
							decision = Erase;
						}
						else
						{
							decision = Next;
						}
					}

					if (decision == Erase)
					{
						auto&& promise = (*iter);
						// 如果有任务在等他完成,应该通知之.将其从等待中解放出来
						if (auto awaiter = promise->m_awaiter)
						{
							auto awaiter_iter = std::find_if(m_waiting_list.begin(), m_waiting_list.end(), 
								[awaiter](std::shared_ptr<BasicPromise> const& spAwaiter) { return spAwaiter.get() == awaiter; }
								);
							if (awaiter_iter != m_waiting_list.end())
							{
								// 将 awaiter_iter 从 m_waiting_list 移动到 m_active_list 尾部
								m_active_list.splice(m_active_list.end(), m_waiting_list, awaiter_iter);
								awaiter->m_waiting = false;
								promise->m_awaiter = nullptr;
							}
							else
							{
								throw std::logic_error("awaitee done but awaiter not found");
							}
						}
						iter = m_active_list.erase(iter);
					}
					else if (decision == Next)
					{
						++iter;
					}
					else if (decision == MoveToWaitList)
					{
						auto next = iter;
						++next;
						{
							// 将 iter 从 m_active_list 移动到 m_waiting_list 尾部
							m_waiting_list.splice(m_waiting_list.end(), m_active_list, iter);
						}
						iter = next;
					}
					else
					{
						throw std::logic_error("unknown decision");
					}
				}
			}
		}

	private:
		void Submit(std::shared_ptr<BasicPromise> const& promise)
		{
			std::lock_guard<std::mutex> lock{ m_add_list_mtx };
			m_add_list.push_back(promise);
		}

	private:
		Duration m_ration;
		std::mutex m_add_list_mtx;
		std::list<std::shared_ptr<BasicPromise>> m_active_list;
		std::list<std::shared_ptr<BasicPromise>> m_add_list;
		std::list<std::shared_ptr<BasicPromise>> m_waiting_list;
	};

	/*  _____________________________  */
	/*             定 义               */
	/*  _____________________________  */

	template<typename Result>
	template<typename InnerTaskResult>
	inline TaskAwaitable<Result, InnerTaskResult> Promise<Result>::await_transform(Task<InnerTaskResult> const& subtask)
	{
		return { this };
	}

	template<typename InnerTaskResult>
	inline TaskAwaitable<void, InnerTaskResult> Promise<void>::await_transform(Task<InnerTaskResult> const& subtask)
	{
		return { this };
	}

	inline BasicPromise::BasicPromise(p<Scheduler> scheduler)
	{
		scheduler->Submit(shared_from_this());
	}

	template<typename OuterTaskResult>
	inline Promise<OuterTaskResult>::Promise(p<Scheduler> scheduler)
		: BasicPromise(scheduler)
	{}

	inline Promise<void>::Promise(p<Scheduler> scheduler)
		: BasicPromise(scheduler)
	{}

	inline Task<void> Promise<void>::get_return_object()
	{
		return Task<void> { std::coroutine_handle<Promise>::from_promise(*this) };
	}

	template<typename Result>
	inline Task<Result> Promise<Result>::get_return_object()
	{
		return Task<Result>{ std::coroutine_handle<Promise>::from_promise(*this) };
	}

	template<typename OuterTaskResult, typename InnerTaskResult>
	inline InnerTaskResult TaskAwaitable<OuterTaskResult, InnerTaskResult>::await_resume()
	{
		switch (m_promise->State())
		{
		case PromiseState::NotDone:
		{
			throw std::logic_error{ "The awaited task has not yet been finished, but the awaiting task is being resumed. Is there a bug in the scheduler?" };
		}
		break;
		case PromiseState::HasException:
		{
			// rethrow exception
			std::exception_ptr exception = m_promise->m_exception;
			throw exception;
		}
		break;
		case PromiseState::HasResult:
		{
			OuterTaskResult result{ std::move(m_promise->m_result) };
			return result;
		}
		break;
		default:
		{
			throw std::logic_error{ "unknown PromiseState" };
		}
		}
	}

	template<typename OuterTaskResult>
	inline void TaskAwaitable<OuterTaskResult, void>::await_resume()
	{
		switch (m_promise->State())
		{
		case PromiseState::NotDone:
		{
			throw std::logic_error{ "The awaited task has not yet been finished, but the awaiting task is being resumed. Is there a bug in the scheduler?" };
		}
		break;
		case PromiseState::HasException:
		{
			// rethrow exception
			std::exception_ptr exception = m_promise->m_exception;
			throw exception;
		}
		break;
		case PromiseState::HasResult:
		{
			return;
		}
		break;
		default:
		{
			throw std::logic_error{ "unknown PromiseState" };
		}
		}
	}
}

using namespace mt;

Task<int> task2(p<Scheduler> scheduler)
{
	co_return 1;
}

int main()
{
	using namespace std::chrono_literals;
	Scheduler scheduler{ 2s };
	task2(&scheduler);
	scheduler.Run(); 
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
