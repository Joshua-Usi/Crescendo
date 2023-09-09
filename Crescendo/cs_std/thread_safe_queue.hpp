#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

// Crescendo custom standard library
namespace cs_std
{
	/// <summary>
	/// Threadsafe queue class for use in multithreaded applications
	/// </summary>
	/// <typeparam name="T">Type</typeparam>
	template<typename T>
	class thread_safe_queue
	{
	private:
		std::queue<T> queue;
		mutable std::mutex mutex;
		std::condition_variable condition;
		bool unlocked = false;
	public:
		thread_safe_queue() = default;
		~thread_safe_queue() = default;
		thread_safe_queue(const thread_safe_queue<T>& other) = delete;
		thread_safe_queue<T>& operator=(const thread_safe_queue<T>& other) = delete;
		/// <summary>
		/// Add a new value to the queue
		/// </summary>
		/// <param name="value">Value to push</param>
		void push(const T& value)
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			this->queue.push(value);
			this->condition.notify_one();
		}
		/// <summary>
		/// Add a new value to the queue using move semantics
		/// </summary>
		/// <param name="value"></param>
		void push(T&& value)
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			this->queue.push(std::move(value));
			this->condition.notify_one();
		}
		/// <summary>
		/// Peek the front of the queue
		/// </summary>
		/// <returns>Front of the queue</returns>
		T front() const
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			return this->queue.front();
		}
		/// <summary>
		/// Pop a value from the queue, this will block until a value is available
		/// Or until the queue is unlocked
		/// If the queue is empty when it is unlocked, then the return value will be default constructed
		/// </summary>
		/// <returns></returns>
		T pop()
		{
			std::unique_lock<std::mutex> lock(this->mutex);
			this->condition.wait(lock, [this]() { return !this->queue.empty() || unlocked; });
			if (this->queue.empty()) return T();
			T value = std::move(this->queue.front());
			this->queue.pop();
			return value;
		}
		/// <summary>
		/// Determine if the queue is empty
		/// The value from this function can get immediately invalidated
		/// At any moment
		/// </summary>
		/// <returns>If the queue is empty</returns>
		bool empty() const
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			return this->queue.empty();
		}
		/// <summary>
		/// Return the size of the queue
		/// The value from this function can get immediately invalidated
		/// At any moment
		/// </summary>
		/// <returns>Size of the queue</returns>
		size_t size() const
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			return this->queue.size();
		}
		void unlock()
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			this->unlocked = true;
			this->condition.notify_all();
		}
	};
}