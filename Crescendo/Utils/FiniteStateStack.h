#pragma once

#include <vector>

#include "core/core.h"

namespace Crescendo {

	template <typename state>
	class FiniteStateStack
	{
	private:
		std::vector<state> stackState;
		std::map<state, std::string> stateMap;
	public:
		FiniteStateStack(state initialState, std::map<state, std::string> map)
		{
			stackState.push_back(initialState);
			stateMap = map;
		}
		state GetPreviousState(int n)
		{
			return stackState[stackState.size() - (n + 1)];
		}
		state GetState()
		{
			return stackState[stackState.size() - 1];
		}
		bool StateExists(state stat)
		{
			for (int i = 0; i < stackState.size(); i++)
			{
				if (stackState[i] == stat) return true;
			}
			return false;
		}
		void Push(state stat)
		{
			stackState.push_back(stat);
		}
		void Pop(int pops = 1)
		{
			for (int i = 0; i < pops; i++)
			{
				stackState.pop_back();
			}
		}
	};
}