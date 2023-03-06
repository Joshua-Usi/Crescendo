#pragma once

#include "core/core.h"
#include "VertexArray.h"
#include "ShaderProgram.h"

#include <memory>

namespace Crescendo::Rendering
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None,
			OpenGL,
			//Vulkan,
			//DirectX11,
			//DirectX12,
			//Metal,
		};
	public:
		/// <summary>
		/// Sets whether or not depth testing is enabled
		/// </summary>
		/// <param name="state">Depth testing state</param>
		virtual void SetDepthTest(bool state) = 0;
		/// <summary>
		/// Sets the clear colour of the screen
		/// </summary>
		/// <param name="r">Red parameter</param>
		/// <param name="g">Green parameter</param>
		/// <param name="b">Blue parameter</param>
		/// <param name="a">Alpha parameter, is optional and defaults to 1.0f</param>
		virtual void SetClear(float r, float g, float b, float a) = 0;
		/// <summary>
		/// Clear the screen with a magenta colur (1.0f, 0.0f, 1.0f, 1.0f)
		/// </summary>
		virtual void Clear() = 0;
		
		/// <summary>
		/// Draw an object to the screen
		/// </summary>
		/// <param name="vertexArray">vertexArray to draw</param>
		virtual void DrawIndexed(const std::shared_ptr<Rendering::VertexArray>& vertexArray) = 0;

		// Set the renderingAPI to use
		static void SetAPI(RendererAPI::API renderingAPI);
		// Get the set rendereringAPI
		static API GetAPI();
	private:
		static API api;
	};
}