#pragma once
#include "common.hpp"
#include "Component.hpp"
#include "../Entity.hpp"

CS_NAMESPACE_BEGIN
{
	struct Behaviour
	{
		virtual void OnUpdate(double time) {}
		virtual void OnLateUpdate(double time) {}
		virtual void OnAttach(Entity e) {}
		virtual void OnDetach(Entity e) {}
	};

	struct Behaviours : public Component
	{
		std::vector<std::unique_ptr<Behaviour>> behaviours;

		Behaviours() = default;
		Behaviours(std::vector<std::unique_ptr<Behaviour>> behaviours) : behaviours(std::move(behaviours)) {}
		template<typename ...T>
		Behaviours(T&&... behaviours)
		{
			(this->behaviours.push_back(std::move(behaviours)), ...);
		}
		Behaviours(const Behaviours&) = delete;
		Behaviours& operator=(const Behaviours&) = delete;
		Behaviours(Behaviours&& other) noexcept : behaviours(std::move(other.behaviours)) {}
		Behaviours& operator=(Behaviours&& other) noexcept
		{
			if (this != &other)
				behaviours = std::move(other.behaviours);
			return *this;
		}
		Behaviours& AddBehaviour(std::unique_ptr<Behaviour> behaviour)
		{
			behaviours.push_back(std::move(behaviour));
			return *this;
		}
		Behaviours& AddBehaviour(Behaviour* behaviour)
		{
			behaviours.push_back(std::unique_ptr<Behaviour>(behaviour));
			return *this;
		}
		// Pre-render
		void OnUpdate(double time)
		{
			for (auto& behaviour : behaviours)
				behaviour->OnUpdate(time);
		}
		// Post-render
		void OnLateUpdate(double time)
		{
			for (auto& behaviour : behaviours)
				behaviour->OnLateUpdate(time);
		}
		void OnAttach(Entity e)
		{
			for (auto& behaviour : behaviours)
				behaviour->OnAttach(e);
		}
		void OnDetach(Entity e)
		{
			for (auto& behaviour : behaviours)
				behaviour->OnDetach(e);
		}
	};
}