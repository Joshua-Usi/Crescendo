#pragma once

#include "core/core.h"

#include "GraphicsAPI.h"

namespace Crescendo::Rendering
{
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() {}

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;

		static GraphicsContext* Create(void* windowHandle);
			
	};
}