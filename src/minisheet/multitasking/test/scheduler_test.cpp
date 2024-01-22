#include "multitasking.h"
#include <gtest/gtest.h>
#include <iostream>
#include "foundation.h"

class SchedTest : public testing::Test
{};

TEST_F(SchedTest, T1)
{
	using namespace std::chrono_literals;
	minisheet::Scheduler scheduler{1ms};
	scheduler.Submit(
		[]() -> minisheet::Task
		{
			for (int i = 0; i < 10000; i++)
			{
				std::cout << "Task A"
						  << ", Step " << i << "\n";
				co_await minisheet::Schedule{};
			}
		});
	scheduler.Submit(
		[]() -> minisheet::Task
		{
			for (int i = 0; i < 20000; i++)
			{
				std::cout << "Task B"
						  << ", Step " << i << "\n";
				co_await minisheet::Schedule{};
			}
		});
	scheduler.Submit(
		[]() -> minisheet::Task
		{
			for (int i = 0; i < 30000; i++)
			{
				std::cout << "Task C"
						  << ", Step " << i << "\n";
				co_await minisheet::Schedule{};
			}
		});
	// scheduler.Submit(
	// 	[]() -> minisheet::mt::Task
	// 	{
	// 		for (int i = 0; 1; i++)
	// 		{
	// 			throw minisheet::NullPointerException();
	// 		}
	// 		co_return;
	// 	});
	scheduler.Run();
}