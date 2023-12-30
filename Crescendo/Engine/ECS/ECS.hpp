#pragma once

#include "common.hpp"

#define ENTT_NOEXCEPTION
#include "libraries/Thirdparty/entt/entt.hpp"

#include <utility>

CS_NAMESPACE_BEGIN
{
	struct Entity
	{
	private:
		friend class EntityManager;
		// reference to the registry of EntityManager
		entt::registry* registry;
		entt::entity entity;
	public:
		Entity(entt::registry* registry, entt::entity entity) : registry(registry), entity(entity) {};

		operator entt::entity& () { return this->entity; };
		uint32_t GetID() const { return static_cast<uint32_t>(this->entity); }

		template<typename T> T& GetComponent() { return registry->get<T>(this->entity); }
		template<typename T> bool HasComponent() const { return registry->all_of<T>(this->entity); }
		template<typename ...T> bool HasComponents() const { return registry->all_of<T...>(this->entity); }
		template<typename T> T& AddComponent(const T& component) { return registry->emplace<T>(this->entity, component); };
		template<typename T, typename... Args> T& EmplaceComponent(Args&&... args) { return registry->emplace<T>(this->entity, std::forward<Args>(args)...); };
		template<typename T> void RemoveComponent() { registry->remove<T>(this->entity); }
	};

	class EntityManager
	{
	private:
		friend struct Entity;
		entt::registry registry;
	public:
		EntityManager() : registry({}) {};

		Entity CreateEntity() { return Entity(&registry, registry.create()); }
		void DestroyEntity(Entity entity) { registry.destroy(entity); }
	};
}