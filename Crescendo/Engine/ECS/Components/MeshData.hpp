#pragma once

#include "common.hpp"
#include "Component.hpp"
#include "cs_std/graphics/algorithms.hpp"
#include "Rendering/Vulkan/ResourceManager.hpp"

CS_NAMESPACE_BEGIN
{
	struct MeshData : public Component
	{
		cs_std::graphics::bounding_aabb bounds; // Used for mesh culling
		Vulkan::MeshHandle meshHandle;

		MeshData() : bounds(), meshHandle() {}
		MeshData(const cs_std::graphics::bounding_aabb& bounds, Vulkan::MeshHandle meshID) : bounds(bounds), meshHandle(meshID) {}
	};
}