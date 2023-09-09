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
		/// <summary>
		/// Add a task to the queue, will immediately be consumed by a thread when avaliable
		/// </summary>
		/// <param name="function">Function to execute</param>
		void AddTask(const std::function<void()> function);
		/// <summary>
		/// Block the main thread until all tasks have been completed
		/// </summary>
		void WaitTillFinished();
		/// <summary>
		/// Non blocking that determines if the queue is empty and all threads are idle
		/// </summary>
		inline bool IsFinished() const { return this->tasks.empty() && this->activeThreads == 0; }
		/// <summary>
		/// Get the number of threads
		/// </summary>
		inline uint32_t GetThreadCount() const { return this->threads.size(); }
		/// <summary>
		/// Get the number of active threads (Thread executing tasks)
		/// </summary>
		inline uint32_t GetActiveThreadCount() const { return this->activeThreads; }
		/// <summary>
		/// Gets the number of pending tasks in the queue
		/// </summary>
		inline uint32_t GetTaskCount() const { return this->tasks.size(); }
	};
}