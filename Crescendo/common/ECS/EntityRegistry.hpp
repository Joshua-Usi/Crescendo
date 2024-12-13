#pragma once
#include "entt/entt.hpp"
#include "Component.hpp"
#include "Entity.hpp"

namespace CrescendoEngine
{
	class EntityRegistry
	{
	private:
		entt::registry m_Registry;
	public:
		EntityRegistry() = default;
		~EntityRegistry() = default;
		// Creates a new entity and returns its handle.
		Entity CreateEntity()
		{
			return Entity(&m_Registry, m_Registry.create());
		}
		// Destroys the entity and all of its components.
		void DestroyEntity(Entity entity)
		{
			m_Registry.destroy(entity);
		}
		// DEEP clones the entity and all of its components.
		Entity CopyEntity(Entity entity)
		{
			Entity other = CreateEntity();
			for (auto [id, storage] : m_Registry.storage())
			{
				if (storage.contains(entity))
					storage.push(other, storage.value(entity));
			}
			return other;
		}
		// Returns the number of components of type T in the registry.
		template <ValidComponent T>
		size_t GetComponentCount() const
		{
			return m_Registry.view<T>().size();
		}
		// Runs a loop over all entities with all the components in T...
		template<ValidComponent ...T>
		void ForEach(std::function<void(T&...)> func)
		{
			m_Registry.view<T...>().each(func);
		}
		// Runs a loop over all entities with all the components in T..., while also providing a reference to the entity
		template<ValidComponent ...T>
		void ForEach(std::function<void(entt::entity, T&...)> func)
		{
			m_Registry.view<T...>().each(func);
		}
	};
}