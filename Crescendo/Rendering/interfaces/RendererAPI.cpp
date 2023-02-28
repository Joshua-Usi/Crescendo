#include "RendererAPI.h"

namespace Crescendo::Rendering
{
	RendererAPI::API RendererAPI::api = RendererAPI::API::OpenGL;
	void RendererAPI::SetAPI(RendererAPI::API renderingAPI)
	{
		api = renderingAPI;
	}
	RendererAPI::API RendererAPI::GetAPI()
	{
		return api;
	}
}