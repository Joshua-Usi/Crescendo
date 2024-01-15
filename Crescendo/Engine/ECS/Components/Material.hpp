#pragma once

#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	struct Material : public Component
	{
		uint32_t pipelineID, diffuseID, normalID;
		bool isTransparent, isDoubleSided, isShadowCasting;

		Material() : pipelineID(0), diffuseID(0), normalID(0), isTransparent(false), isDoubleSided(false), isShadowCasting(true) {}
		Material(uint32_t pipelineID, uint32_t diffuseID, uint32_t normalID, bool isTransparent, bool isDoubleSided, bool isShadowCasting) : pipelineID(pipelineID), diffuseID(diffuseID), normalID(normalID), isTransparent(isTransparent), isDoubleSided(isDoubleSided), isShadowCasting(isShadowCasting) {}
	};
}