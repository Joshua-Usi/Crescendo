#pragma once

#include "Rendering/Renderer/Renderer.hpp"

namespace Crescendo
{
	class Atlas
	{
	public:
		Renderer renderer;
	public:
		Atlas() = default;
		Atlas(const Renderer::BuilderInfo& info);
		~Atlas();

		Atlas(const Atlas&) = delete;
		Atlas& operator=(const Atlas&) = delete;

		Atlas(Atlas&& other) noexcept = default;
		Atlas& operator=(Atlas&& other) noexcept = default;
	};
}