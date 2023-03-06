#pragma once

#include "RendererAPI.h"

#include <memory>

namespace Crescendo::Rendering
{
	class RenderCommand
	{
	private:
		static RendererAPI* rendererAPI;
	public:
		inline static void SetClear(float r, float g, float b, float a = 1.0f)
		{
			rendererAPI->SetClear(r, g, b, a);
		}
		/// <summary>
		/// Clear the screen with a magenta colour (1.0f, 0.0f, 1.0f, 1.0f)
		/// </summary>
		inline static void Clear()
		{
			rendererAPI->Clear();
		}
		inline static void SetDepthTest(bool state)
		{
			rendererAPI->SetDepthTest(state);
		}
		/// <summary>
		/// Draw an object to the screen
		/// </summary>
		/// <param name="vertexArray">vertexArray to draw</param>
		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			rendererAPI->DrawIndexed(vertexArray);
		}
	};
}