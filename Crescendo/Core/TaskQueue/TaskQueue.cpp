#include "TaskQueue.hpp"

namespace Crescendo
{
	TaskQueue::TaskQueue(uint32_t threadOverride) : isRunning(true)
	{

		// Determine number of threads
		uint32_t threadCount = std::min(threadOverride, std::thread::hardware_concurrency());
		
		// Create threads
		for (uint32_t i = 0; i < threadCount; i++)
		{
			this->threads.emplace_back([this]() {
				while (this->isRunning)
				{
					std::function<void()> func;
					func = tasks.pop();
					if (func)
					{
						activeThreads++;
						func();
						activeThreads--;
					}
				}
			});
		}
	}
	TaskQueue::~TaskQueue()
	{
		this->tasks.unlock();
		this->isRunning = false;
		int i = 0;
		for (auto& thread : this->threads) thread.join();
	}
	void TaskQueue::AddTask(const std::function<void()> function)
	{
		this->tasks.push(function);
	}
	void TaskQueue::WaitTillFinished()
	{
		while (!this->tasks.empty() || this->activeThreads > 0) std::this_thread::yield();
	}
}