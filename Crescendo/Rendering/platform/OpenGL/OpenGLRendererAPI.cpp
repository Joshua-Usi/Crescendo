#include "OpenGLRendererAPI.h"

#include "glad/glad.h"

void Crescendo::Rendering::OpenGLRendererAPI::SetDepthTest(bool state)
{
	if (state)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}

void Crescendo::Rendering::OpenGLRendererAPI::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Crescendo::Rendering::OpenGLRendererAPI::SetClear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void Crescendo::Rendering::OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<Rendering::VertexArray>& vertexArray)
{
	glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
}
