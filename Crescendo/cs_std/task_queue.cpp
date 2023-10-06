#include "task_queue.hpp"

namespace cs_std
{
	task_queue::task_queue(uint32_t threadOverride)
	{
		this->wake(threadOverride);
	}
	task_queue::~task_queue()
	{
		this->sleep();
	}
	void task_queue::sleep()
	{
		if (!this->isRunning) return;
		this->isRunning = false;

		this->tasks.unlock();
		for (auto& thread : this->threads) thread.join();
	}
	void task_queue::wake(uint32_t threadOverride)
	{
		if (this->isRunning) return;
		this->isRunning = true;

		this->threads.clear();
		// Determine number of threads
		uint32_t threadCount = std::min(threadOverride, std::thread::hardware_concurrency());
		// Create threads
		for (uint32_t i = 0; i < threadCount; i++)
		{
			this->threads.emplace_back([this]() {
				while (this->isRunning)
				{
					auto func = tasks.pop();
					if (func.has_value())
					{
						activeThreads++;
						func.value()();
						activeThreads--;
					}
					else
					{
						std::this_thread::yield();
					}
				}
			});
		}
	}
	void task_queue::push_back(const std::function<void()> function)
	{
		this->tasks.push(function);
	}
	void task_queue::wait_till_finished()
	{
		while (!this->finished()) std::this_thread::yield();
	}
}