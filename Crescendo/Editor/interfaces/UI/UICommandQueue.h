#pragma once

#include <vector>

#include "core/core.h"

#include "UICommand.h"

namespace Crescendo::Editor::Slick
{
	class UICommandQueue
	{
	private:
		std::vector<Commands::UICommand*> commands;
	public:
		void Execute()
		{
			for (int i = 0; i < commands.size(); i++)
			{
				commands[i]->Execute();
			}
		}
	};
}