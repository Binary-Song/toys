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

#include "foundation/foundation.h"
#include "multitasking.h"
namespace llama::mt
{

std::suspend_always BasicPromise::final_suspend() noexcept
{
    try
    {
        std::lock_guard<std::mutex> lock{m_result_mtx};
        m_finished = true;
        return {};
    }
    catch (...)
    {
        return {};
    }
}

void BasicPromise::Resume()
{
    BasicCoroutineHandle::from_promise(*this).resume();
}

bool BasicPromise::Done() const
{
    std::lock_guard<std::mutex> lock(m_result_mtx);
    // 不用 std:;coroutine_handle::done 确保线程安全
    return m_finished;
}

PromiseStatus BasicPromise::Status() const
{
    std::lock_guard<std::mutex> lock(m_result_mtx);
    if (m_finished)
    {
        if (m_exception == nullptr)
            return PromiseStatus::HasResult;
        else
            return PromiseStatus::HasException;
    }
    else
    {
        return PromiseStatus::NotDone;
    }
}

ScheduleAwaitable::ScheduleAwaitable(p<BasicPromise> promise) : m_promise{promise}
{
}

bool ScheduleAwaitable::await_ready()
{
    return Now() < m_promise->m_deadline;
}

void ScheduleAwaitable::await_suspend()
{
}

void ScheduleAwaitable::await_resume()
{
}



} // namespace llama::mt
