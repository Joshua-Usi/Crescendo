#pragma once

#include <vector>
#include <functional>

namespace Crescendo::internal
{
	class FunctionQueue
	{
	private:
		std::vector<std::function<void()>> queue;
	public:
		inline void Push(std::function<void()>&& function) { queue.push_back(function); }
		inline void Flush()
		{
			for (auto it = queue.rbegin(); it != queue.rend(); it++) (*it)();
			this->queue.clear();
		}
	};
}