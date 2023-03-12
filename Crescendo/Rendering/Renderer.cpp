#include "Renderer.h"

namespace Crescendo::Rendering
{
	std::unique_ptr<Renderer::SceneData> Renderer::sceneData = std::make_unique<Renderer::SceneData>();
	void Renderer::BeginScene(PerspectiveCamera& camera)
	{
		sceneData->viewProjectionMatrix = camera.GetViewProjectionMatrix();
	}
	void Renderer::EndScene()
	{

	}
	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		shader->SetMat4("uProjectionView", sceneData->viewProjectionMatrix);
		shader->SetMat4("uModel", transform);
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
	RendererAPI::API Renderer::GetAPI()
	{
		return RendererAPI::GetAPI();
	}
}