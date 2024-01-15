#pragma once

#include "common.hpp"
#include "Component.hpp"
#include <limits>

CS_NAMESPACE_BEGIN
{
	struct Skybox : public Component
	{
		uint32_t meshID, textureID;

		Skybox() : meshID(std::numeric_limits<uint32_t>::max()), textureID(std::numeric_limits<uint32_t>::max()) {}
		Skybox(uint32_t meshID, uint32_t textureID) : meshID(meshID), textureID(textureID) {}
	};
}