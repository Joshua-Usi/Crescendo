#pragma once

#include "interfaces/RendererAPI.h"

namespace Crescendo::Rendering
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<Rendering::VertexArray>& vertexArray) override;
	};
}