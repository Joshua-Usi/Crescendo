#pragma once

#include "common.hpp"

#define ENTT_NOEXCEPTION
#include "libraries/Thirdparty/entt/entt.hpp"

#include <utility>

#include "Components.hpp"

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

		template<ValidComponent T> T& GetComponent() { return registry->get<T>(this->entity); }
		template<ValidComponent T> bool HasComponent() const { return registry->all_of<T>(this->entity); }
		// Return true if the entity has all of the components, false if it has any less
		template<ValidComponent ...T> bool HasComponents() const { return registry->all_of<T...>(this->entity); }
		// Return true if the entity has any of the components, false if it has none
		template<ValidComponent ...T> bool HasAnyComponents() const { return registry->any_of<T...>(this->entity); }
		template<ValidComponent T> T& AddComponent(const T& component) { return registry->emplace<T>(this->entity, component); };
		template<ValidComponent T, typename... Args> T& EmplaceComponent(Args&&... args) { return registry->emplace<T>(this->entity, std::forward<Args>(args)...); };
		template<ValidComponent T> void RemoveComponent() { registry->remove<T>(this->entity); }
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