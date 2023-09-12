#include "Atlas.hpp"

#include "Engine/CVar/Cvar.hpp"

namespace Crescendo
{
	Atlas::Atlas(const Renderer::BuilderInfo& info)
	{
		this->renderer = Renderer::Create(info);
	}

	Atlas::~Atlas()
	{
		Renderer::Destroy(this->renderer);
	}
}
