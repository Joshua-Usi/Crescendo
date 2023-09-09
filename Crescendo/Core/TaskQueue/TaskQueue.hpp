#pragma once

#include <queue>
#include <thread>
#include <functional>
#include <atomic>

#include "Core/common.hpp"

#include "cs_std/thread_safe_queue.hpp"

namespace Crescendo
{
	class TaskQueue
	{
	private:
		cs_std::thread_safe_queue<std::function<void()>> tasks;
		std::vector<std::thread> threads;
		std::atomic<bool> isRunning;
		std::atomic<uint32_t> activeThreads;
	public:
		explicit TaskQueue(uint32_t threadOverride = std::numeric_limits<uint32_t>::max());
		~TaskQueue();

		// Delete move
		TaskQueue(TaskQueue&&) = delete;
		TaskQueue& operator=(TaskQueue&&) = delete;

		// Delete copy
		TaskQueue(const TaskQueue&) = delete;
		TaskQueue& operator=(const TaskQueue&) = delete;

		void AddTask(const std::function<void()> function);

		void WaitTillFinished();
	};
}