#pragma once

#include "GraphicsAPI.h"

namespace Crescendo::Rendering
{
	class Renderer
	{
	public:
		inline static GraphicsAPI GetAPI() { return chosenAPI; };

		inline static void SetAPI(GraphicsAPI api) { chosenAPI = api; };
	private:
		static GraphicsAPI chosenAPI;
	};
}