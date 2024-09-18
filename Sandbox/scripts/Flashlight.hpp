#pragma once
#include "Crescendo.hpp"
using namespace CrescendoEngine;

class Flashlight : public Behaviour
{
public:
	SpotLight* light;

	virtual void OnAttach(Entity e) override
	{
		light = &e.GetComponent<SpotLight>();
	}
	virtual void OnUpdate(double dt) override
	{
		Input* input = Application::Get()->GetWindow()->GetInput();
		if (input->GetKeyDown(Key::F))
			light->intensity = (light->intensity == 0.0f) ? 10.0f : 0.0f;
	}
};
