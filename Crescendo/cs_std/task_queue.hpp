#pragma once

#include <thread>
#include <functional>
#include <atomic>

#include "thread_safe_queue.hpp"

namespace cs_std
{
	/// <summary>
	/// Safe multithreaded task queue designed to automatically assign tasks to threads
	/// </summary>
	class task_queue
	{
	private:
		cs_std::thread_safe_queue<std::function<void()>> tasks;
		std::vector<std::thread> threads;
		std::atomic<bool> isRunning;
		std::atomic<uint32_t> activeThreads;
	public:
		explicit task_queue(uint32_t threadOverride = std::numeric_limits<uint32_t>::max());
		~task_queue();
		// Delete move
		task_queue(task_queue&&) = delete;
		task_queue& operator=(task_queue&&) = delete;
		// Delete copy
		task_queue(const task_queue&) = delete;
		task_queue& operator=(const task_queue&) = delete;
		/// <summary>
		/// Put the queue to sleep, all threads will finish their current task and then stopped and the queue will be in a paused state
		/// </summary>
		void sleep();
		/// <summary>
		/// Wake the queue, threads will be started up again and the queue will be in a running state
		/// </summary>
		void wake(uint32_t threadOverride = std::numeric_limits<uint32_t>::max());
		/// <summary>
		/// Add a task to the queue, will immediately be consumed by a thread when availiable
		/// </summary>
		/// <param name="function">Function to execute</param>
		void push_back(const std::function<void()> function);
		/// <summary>
		/// Block the main thread until all tasks have been completed
		/// </summary>
		void wait_till_finished();
		/// <summary>
		/// Non blocking that determines if the queue is empty and all threads are idle
		/// </summary>
		inline bool finished() const { return this->tasks.empty_unsafe() && this->activeThreads == 0; }
		/// <summary>
		/// Get the number of threads
		/// </summary>
		inline uint32_t thread_count() const { return this->threads.size(); }
		/// <summary>
		/// Get the number of active threads (Thread executing tasks)
		/// </summary>
		inline uint32_t active_thread_count() const { return this->activeThreads; }
		/// <summary>
		/// Gets the number of pending tasks in the queue
		/// </summary>
		inline uint32_t size() const { return this->tasks.size_unsafe(); }
	};
}