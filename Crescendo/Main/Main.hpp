#pragma once
#include "Interfaces/Module.hpp"
#include "Console.hpp"
#include "Core.hpp"

using namespace CrescendoEngine;

struct DummyComponent : public Component
{
	float x;

	DummyComponent(float x) : x(x) {}
};

class Main : public Module
{
public:
	void OnLoad() override
	{
		EntityRegistry& registry = Core::Get()->GetEntityRegistry();

		Entity entity = registry.CreateEntity();
		entity.EmplaceComponent<DummyComponent>(5.0f);

	}
	void OnUnload() override
	{

	}
	void OnUpdate(double dt) override
	{
		Console::Log("Update.....");

		EntityRegistry& registry = Core::Get()->GetEntityRegistry();
		Console::Log("Component count: ", registry.GetComponentCount<DummyComponent>());
		registry.ForEach<DummyComponent>([&](DummyComponent& component) {
			Console::Log(component.x);
		});
	}
	static ModuleMetadata GetMetadata()
	{
		return
		{
			"Main",
			"0.0.1",
			"Main module for Crescendo",
			"Joshua Usi",
			"",
			0.5
		};
	}
};