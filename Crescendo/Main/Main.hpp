#pragma once
#include "Interfaces/Module.hpp"
#include "Console.hpp"
#include "Core.hpp"

#include "../WindowManager/WindowManager.hpp"
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

		Module* windowManager = Core::Get()->GetModule("WindowManager");

		dynamic_cast<WindowManagerInterface*>(windowManager)->CreateWindow(800, 600, "Test");

	}
	void OnUnload() override
	{

	}
	void OnUpdate(double dt) override
	{
		Console::Log("Update.....");
	}
	static ModuleMetadata GetMetadata()
	{
		return
		{
			"Main",
			"0.0.1",
			"Main module for Crescendo",
			"Joshua Usi",
			"WindowManager",
			0.5
		};
	}
};