#include "Renderer.h"

namespace Crescendo::Rendering
{
	void Renderer::BeginScene()
	{

	}
	void Renderer::EndScene()
	{

	}
	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray)
	{
		RenderCommand::DrawIndexed(vertexArray);
	}
	RendererAPI::API Renderer::GetAPI()
	{
		return RendererAPI::GetAPI();
	}
}