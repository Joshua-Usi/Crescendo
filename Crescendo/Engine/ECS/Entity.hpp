#pragma once

#include "common.hpp"

#define ENTT_NOEXCEPTION
#include "libraries/Thirdparty/entt/entt.hpp"

#include <utility>
#include <functional>

#include "Components/Component.hpp"

CS_NAMESPACE_BEGIN
{
	struct Behaviours;

	struct Entity
	{
	private:
		friend class EntityManager;
		// reference to the registry of EntityManager
		entt::registry* registry;
		entt::entity entity;
	public:
		Entity() : registry(nullptr), entity(entt::null) {};
		Entity(entt::registry* registry, entt::entity entity) : registry(registry), entity(entity) {};
		Entity(const Entity& other) : registry(other.registry), entity(other.entity) {};
		Entity(Entity&& other) noexcept : registry(other.registry), entity(other.entity) {};
		Entity& operator=(const Entity& other) { this->registry = other.registry; this->entity = other.entity; return *this; };
		Entity& operator=(Entity&& other) noexcept { this->registry = other.registry; this->entity = other.entity; return *this; };

		operator entt::entity& () { return this->entity; };
		uint32_t GetID() const { return static_cast<uint32_t>(this->entity); }
		bool IsValid() const { return registry->valid(this->entity); }

		template<ValidComponent T> T& GetComponent() { return registry->get<T>(this->entity); }
		template<ValidComponent T> bool HasComponent() const { return registry->all_of<T>(this->entity); }
		// Return true if the entity has all of the components, false if it has any less
		template<ValidComponent ...T> bool HasComponents() const { return registry->all_of<T...>(this->entity); }
		// Return true if the entity has any of the components, false if it has none
		template<ValidComponent ...T> bool HasAnyComponents() const { return registry->any_of<T...>(this->entity); }
		template<ValidComponent T> T& AddComponent(const T& component) { return registry->emplace<T>(this->entity, component); };
		template<ValidComponent T, typename... Args> T& EmplaceComponent(Args&&... args)
		{
			T& component = registry->emplace<T>(this->entity, std::forward<Args>(args)...);
			if constexpr (std::is_same_v<T, Behaviours>) component.OnAttach(*this);
			return component;
		};
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
		void DestroyEntity(Entity& entity) { registry.destroy(entity); }
		Entity CopyEntity(Entity& entity)
		{
			Entity other = CreateEntity();
			for (auto [id, storage] : registry.storage())
			{
				if (storage.contains(entity)) storage.push(other, storage.value(entity));
			}
			return other;
		}
		Entity GetEntity(entt::entity entity) { return Entity(&registry, entity); }
		template<ValidComponent T> size_t GetComponentCount() const { return registry.view<T>().size(); }
		template<ValidComponent ...T> void ForEach(std::function<void(entt::entity, T&...)> func) { registry.view<T...>().each(func); }
		template<ValidComponent ...T> void ForEach(std::function<void(T&...)> func) { registry.view<T...>().each(func); }
	};
}