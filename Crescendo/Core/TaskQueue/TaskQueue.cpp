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
					auto func = tasks.pop();
					if (func.has_value())
					{
						activeThreads++;
						func.value()();
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
		for (auto& thread : this->threads) thread.join();
	}
	void TaskQueue::AddTask(const std::function<void()> function)
	{
		this->tasks.push(function);
	}
	void TaskQueue::WaitTillFinished()
	{
		while (!this->IsFinished()) std::this_thread::yield();
	}
}