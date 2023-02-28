#include "OpenGLRendererAPI.h"

#include "glad/glad.h"

void Crescendo::Rendering::OpenGLRendererAPI::Clear()
{
	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Crescendo::Rendering::OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<Rendering::VertexArray>& vertexArray)
{
	vertexArray->Bind();
	glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
}
