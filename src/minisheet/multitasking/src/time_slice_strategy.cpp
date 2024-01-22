#include "multitasking/time_slice_strategy.h"

namespace minisheet
{
namespace mt
{


void DeadlineScheduler::Step()
{
	for (auto &task_item : m_tasks)
	{
		DeadlineTask task = std::get<1>(task_item);
		bool &alive = std::get<0>(task_item);
		task.promise().m_nextSusTime = now() + m_timeSliceDuration;
		task.resume();
		if (task.done())
		{
			task.destroy();
			alive = false;
		}
	}
	m_tasks.erase(
		std::remove_if(
			m_tasks.begin(),
			m_tasks.end(),
			[](const auto &task_item)
			{
				bool alive = std::get<0>(task_item);
				return !alive;
			}),
		m_tasks.end());
}

void DeadlineScheduler::Run()
{
	while (m_tasks.size())
	{
		Step();
	}
}

void DeadlinePromise::unhandled_exception()
{
	m_exception = std::current_exception();
}

}//namespace mt
}//namespace minisheet