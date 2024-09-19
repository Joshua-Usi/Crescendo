#pragma once
#include "Crescendo.hpp"
using namespace CrescendoEngine;

class Flashlight : public Behaviour
{
public:
	SpotLight* light;

	virtual void OnAttach(Entity e) override
	{
		light = &e.EmplaceComponent<SpotLight>(math::vec3(1.0f, 1.0f, 1.0f), 100.0f, math::radians(1.0f), math::radians(25.0f), false);;
	}
	virtual void OnUpdate(double dt) override
	{
		Input* input = Application::Get()->GetWindow()->GetInput();
		if (input->GetKeyDown(Key::F))
			light->intensity = (light->intensity == 0.0f) ? 100.0f : 0.0f;
	}
};
