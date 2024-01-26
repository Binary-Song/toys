#include "multitasking.h"
#include <gtest/gtest.h>
#include <iostream>
#include "foundation.h"

class SchedTest : public testing::Test
{};

TEST_F(SchedTest, T1)
{
	using namespace std::chrono_literals;
	llama::Scheduler scheduler{1ms};
	scheduler.Submit(
		[]() -> llama::Task
		{
			for (int i = 0; i < 10000; i++)
			{
				std::cout << "Task A"
						  << ", Step " << i << "\n";
				co_await llama::Schedule{};
			}
		});
	scheduler.Submit(
		[]() -> llama::Task
		{
			for (int i = 0; i < 20000; i++)
			{
				std::cout << "Task B"
						  << ", Step " << i << "\n";
				co_await llama::Schedule{};
			}
		});
	scheduler.Submit(
		[]() -> llama::Task
		{
			for (int i = 0; i < 30000; i++)
			{
				std::cout << "Task C"
						  << ", Step " << i << "\n";
				co_await llama::Schedule{};
			}
		});
	// scheduler.Submit(
	// 	[]() -> llama::mt::Task
	// 	{
	// 		for (int i = 0; 1; i++)
	// 		{
	// 			throw llama::NullPointerException();
	// 		}
	// 		co_return;
	// 	});
	scheduler.Run();
}