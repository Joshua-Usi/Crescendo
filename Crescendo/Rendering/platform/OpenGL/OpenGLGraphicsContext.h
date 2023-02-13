#pragma once

#include "interfaces/GraphicsContext.h"

struct GLFWwindow;

namespace Crescendo::Rendering
{
	class OpenGLGraphicsContext : public GraphicsContext
	{
	private:
		GLFWwindow* windowHandle;
	public:
		OpenGLGraphicsContext(GLFWwindow* windHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;
	};
}