#pragma once

#include "common.hpp"
#include "Component.hpp"

#include "cs_std/graphics/algorithms.hpp"

CS_NAMESPACE_BEGIN
{
	struct MeshData : public Component
	{
		cs_std::graphics::bounding_aabb bounds;
		uint32_t meshID;

		MeshData() : bounds(), meshID(0) {}
		MeshData(const cs_std::graphics::bounding_aabb& bounds, uint32_t meshID) : bounds(bounds), meshID(meshID) {}
	};
}