#pragma once
#include "common.hpp"
#include "Component.hpp"
#include "cs_std/graphics/algorithms.hpp"
#include "Rendering/ResourceHandles.hpp"

CS_NAMESPACE_BEGIN
{
	struct MeshData : public Component
	{
		cs_std::graphics::bounding_aabb bounds; // Used for mesh culling
		Vulkan::Mesh meshHandle;

		MeshData() : bounds(), meshHandle() {}
		MeshData(const cs_std::graphics::bounding_aabb& bounds, Vulkan::Mesh mesh) : bounds(bounds), meshHandle(mesh) {}
	};
}