#pragma once

#include "common.hpp"
#include "Engine/ECS/ECS.hpp"
#include "cs_std/packed_vector.hpp"

CS_NAMESPACE_BEGIN
{
	class Scene
	{
	public:
		EntityManager entityManager;
		cs_std::packed_vector<Entity> entities;
		Entity activeCamera;
		Scene() : activeCamera() {}
		void LoadModels(std::vector<cs_std::graphics::model>& models);
	};
}