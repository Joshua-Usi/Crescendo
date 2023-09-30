#pragma once

#include <vector>
#include <functional>

namespace cs_std
{
	class function_queue
	{
	private:
		std::vector<std::function<void()>> queue;
	public:
		inline void push(std::function<void()>&& function) { queue.push_back(function); }
		inline void flush()
		{
			for (auto it = queue.rbegin(); it != queue.rend(); it++) (*it)();
			this->queue.clear();
		}
	};
}