#pragma once

#include "interfaces/RendererAPI.h"

namespace Crescendo::Rendering
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void SetDepthTest(bool state) override;

		virtual void SetClear(float r, float g, float b, float a) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<Rendering::VertexArray>& vertexArray) override;
	};
}