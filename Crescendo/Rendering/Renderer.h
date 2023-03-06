#pragma once

#include "interfaces/RendererAPI.h"
#include "interfaces/RenderCommand.h"
#include "cameras/Perspective/PerspectiveCamera.h"

namespace Crescendo::Rendering
{
	class Renderer
	{
	private:
		struct SceneData
		{
			glm::mat4 viewProjectionMatrix;
		};

		static std::unique_ptr<SceneData> sceneData;
	public:
		/// <summary>
		/// Setup the scene
		/// </summary>
		static void BeginScene(PerspectiveCamera& camera);
		/// <summary>
		/// Finish and cleanup the scene
		/// </summary>
		static void EndScene();

		/// <summary>
		/// Submit geometry to be drawn
		/// </summary>
		/// <param name="vertexArray">vertexArray to be drawn</param>
		static void Submit(const std::shared_ptr<ShaderProgram>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		/// <summary>
		/// Get the currently set API
		/// </summary>
		/// <returns>The Set API as an enum</returns>
		static RendererAPI::API GetAPI();
	};
}