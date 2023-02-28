#include "RenderCommand.h"

#include "platform/OpenGL/OpenGLRendererAPI.h"

namespace Crescendo::Rendering
{
	RendererAPI* RenderCommand::rendererAPI = new OpenGLRendererAPI();
}