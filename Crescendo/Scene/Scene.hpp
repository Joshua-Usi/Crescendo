#pragma once

#include "common.hpp"
#include "ECS/ECS.hpp"
#include "cs_std/packed_vector.hpp"

CS_NAMESPACE_BEGIN
{
	class Scene
	{
	friend class Application;
	friend class Renderer;
	protected:
		EntityManager entityManager;
		cs_std::packed_vector<Entity> entities;
		Entity activeCamera;
		Vulkan::TextureHandle skybox;
	public:
		Scene() : activeCamera() {}
		void LoadModels(std::vector<cs_std::graphics::model>& models);
		void SetActiveCamera(Entity& camera);
		Entity& GetActiveCamera();
		void SetSkybox(Vulkan::TextureHandle skyboxHandle);
		Vulkan::TextureHandle GetSkybox();
		Entity CreateEntity();
	};
}