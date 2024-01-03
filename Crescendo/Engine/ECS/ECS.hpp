#pragma once

#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	struct Component {};

	template<typename T>
	concept ValidComponent = std::derived_from<T, Component>;
}

#include "Entity.hpp"
#include "Components.hpp"