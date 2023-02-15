#pragma once

#include "core/core.h"

#include "GraphicsAPI.h"

namespace Crescendo::Rendering
{
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() {}

		/// <summary>
		/// Initialises the graphics engine
		/// </summary>
		virtual void Init() = 0;
		/// <summary>
		/// Swaps the front buffer with the back buffer
		/// </summary>
		virtual void SwapBuffers() = 0;

		/// <summary>
		/// Creates a RenderAPI specific GraphicsContext
		/// </summary>
		/// <returns>A polymorphic GraphicsContext</returns>
		static GraphicsContext* Create(void* windowHandle);
			
	};
}