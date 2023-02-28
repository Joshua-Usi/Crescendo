#pragma once

#include "interfaces/RendererAPI.h"
#include "interfaces/RenderCommand.h"

namespace Crescendo::Rendering
{
	class Renderer
	{
	public:
		/// <summary>
		/// Setup the scene
		/// </summary>
		static void BeginScene();
		/// <summary>
		/// Finish and cleanup the scene
		/// </summary>
		static void EndScene();

		/// <summary>
		/// Submit geometry to be drawn
		/// </summary>
		/// <param name="vertexArray">vertexArray to be drawn</param>
		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);

		/// <summary>
		/// Get the currently set API
		/// </summary>
		/// <returns>The Set API as an enum</returns>
		static RendererAPI::API GetAPI();
	};
}