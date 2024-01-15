#pragma once

#include "common.hpp"
#include "Component.hpp"

#include <string>

CS_NAMESPACE_BEGIN
{
	struct Name : public Component
	{
		std::string name;
		Name(const std::string& name) : name(name) {}
		Name(const char* name) : name(name) {}
	};
}