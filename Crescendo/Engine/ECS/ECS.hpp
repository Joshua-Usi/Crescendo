#pragma once

#include "libraries/Thirdparty/entt/entt.hpp"

namespace Crescendo::Engine
{
	struct Entity
	{
	private:
		friend class EntityManager;
		// reference to the registry of EntityManager
		static entt::registry& registry;
		entt::entity entity;
	public:
		inline Entity(entt::entity entity) : entity(entity) {};
		inline operator entt::entity& () { return this->entity; };

		template<typename T> T& GetComponent() { return registry.get<T>(this->entity); }
		template<typename T> bool HasComponent() { return registry.all_of<T>(this->entity); }
		template<typename T> T& AddComponent(const T& component) { return registry.emplace<T>(this->entity, component); };
		 template<typename T, typename... Args> T& AddComponent(Args&&... args) { return registry.emplace<T>(this->entity, std::forward<Args>(args)...); };
		template<typename T> void RemoveComponent() { registry.remove<T>(this->entity); }
	};

	class EntityManager
	{
	private:
		friend struct Entity;
		static entt::registry registry;
	public:
		inline static Entity CreateEntity() { return Entity(EntityManager::registry.create()); }
		static void DestroyEntity(Entity entity) { EntityManager::registry.destroy(entity); }
		//template<typename T> static void AddComponent(const Entity entity, const T& component) { EntityManager::registry.emplace<T>(entity, component); }
		//template<typename T, typename... Args> static void AddComponent(const Entity entity, Args&&... args) { EntityManager::registry.emplace<T>(entity, std::forward<Args>(args)...); }
		template<typename T> static void RemoveComponent(const Entity entity) { EntityManager::registry.remove<T>(entity); }
	};
}