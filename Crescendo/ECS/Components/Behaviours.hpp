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
		std::vector<std::shared_ptr<Behaviour>> behaviours;

		Behaviours() : behaviours() {}
		Behaviours(std::vector<std::shared_ptr<Behaviour>> behaviours) : behaviours(behaviours) {}
		template<typename ...T> Behaviours(T... behaviours) : behaviours({ behaviours... }) {}

		Behaviours& AddBehaviour(std::shared_ptr<Behaviour> behaviour) { behaviours.push_back(behaviour); return *this; }
		Behaviours& AddBehaviour(Behaviour* behaviour) { behaviours.push_back(std::shared_ptr<Behaviour>(behaviour)); return *this; }

		// Pre-render
		void OnUpdate(double time) { for (auto& behaviour : behaviours) behaviour->OnUpdate(time); }
		// Post-render
		void OnLateUpdate(double time) { for (auto& behaviour : behaviours) behaviour->OnLateUpdate(time); }
		void OnAttach(Entity e) { for (auto& behaviour : behaviours) behaviour->OnAttach(e); }
		void OnDetach(Entity e) { for (auto& behaviour : behaviours) behaviour->OnDetach(e); }
	};
}