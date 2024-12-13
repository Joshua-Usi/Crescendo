#pragma once
#include <concepts>

namespace CrescendoEngine
{
	// Module components must inherit from this struct.
	struct Component {};

	template<typename T>
	concept ValidComponent = std::derived_from<T, Component>;
}