#pragma once
#include "Component.hpp"
#include "entt/entt.hpp"

namespace CrescendoEngine
{
	class Entity
	{
	private:
		entt::registry* m_Registry;
		entt::entity m_Entity;
	public:
		Entity() : m_Registry(nullptr), m_Entity(entt::null) {};
		Entity(entt::registry* m_Registry, entt::entity m_Entity) : m_Registry(m_Registry), m_Entity(m_Entity) {};
		Entity(const Entity& other) : m_Registry(other.m_Registry), m_Entity(other.m_Entity) {};
		Entity(Entity&& other) noexcept : m_Registry(other.m_Registry), m_Entity(other.m_Entity) {};
		Entity& operator=(const Entity& other) { this->m_Registry = other.m_Registry; this->m_Entity = other.m_Entity; return *this; };
		Entity& operator=(Entity&& other) noexcept { this->m_Registry = other.m_Registry; this->m_Entity = other.m_Entity; return *this; };

		operator entt::entity& () { return this->m_Entity; };
		// Returns the underlying identifier of the m_Entity
		uint32_t GetID() const
		{
			return static_cast<uint32_t>(this->m_Entity);
		}
		// Determines if an m_Entity is valid
		bool IsValid() const
		{
			return m_Registry->valid(this->m_Entity);
		}
		// Return a reference to the component, throws an exception if the m_Entity does not have the component
		template<ValidComponent T>
		T& GetComponent()
		{
			return m_Registry->get<T>(this->m_Entity);
		}
		// Return true if the m_Entity has the component, false if it does not
		template<ValidComponent T>
		bool HasComponent() const
		{
			return m_Registry->all_of<T>(this->m_Entity);
		}
		// Return true if the m_Entity has all of the components, false if it has any less
		template<ValidComponent ...T>
		bool HasComponents() const
		{
			return m_Registry->all_of<T...>(this->m_Entity);
		}
		// Return true if the m_Entity has any of the components, false if it has none
		template<ValidComponent ...T>
		bool HasAnyComponents() const
		{
			return m_Registry->any_of<T...>(this->m_Entity);
		}
		// Adds a component to the m_Entity, returns a reference to the component
		template<ValidComponent T>
		T& AddComponent(const T& component)
		{
			return m_Registry->emplace<T>(this->m_Entity, component);
		};
		// Emplaces a component to the m_Entity, returns a reference to the component
		template<ValidComponent T, typename... Args>
		T& EmplaceComponent(Args&&... args)
		{
			T& component = m_Registry->emplace<T>(this->m_Entity, std::forward<Args>(args)...);
			return component;
		};
		// Removes a component from the m_Entity
		template<ValidComponent T>
		void RemoveComponent()
		{
			m_Registry->remove<T>(this->m_Entity);
		}
	};
}