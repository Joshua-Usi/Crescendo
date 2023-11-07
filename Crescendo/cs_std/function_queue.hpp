#pragma once

#include <stack>
#include <functional>

namespace cs_std
{
	class function_queue
	{
	private:
		std::stack<std::function<void()>> stack;
	public:
		/// <summary>
		/// Add a function to the queue, the last function pushed on the queue will be called first
		/// </summary>
		/// <param name="function"></param>
		inline void push(std::function<void()>&& function) { stack.push(std::move(function)); }
		/// <summary>
		/// Flush the queue, the last function pushed on the queue will be called first
		/// </summary>
		inline void flush()
		{
			while (!stack.empty())
			{
				auto& function = stack.top();
				function();
				stack.pop();
			}
		}
		/// <summary>
		/// Clears the queue without calling any functions
		/// </summary>
		inline void clear() { while (!stack.empty()) stack.pop(); }
	};
}