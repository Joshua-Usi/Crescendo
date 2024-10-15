#pragma once
#include "Crescendo.hpp"
using namespace CrescendoEngine;

class Campfire : public Behaviour
{
public:
	PointLight& pointLight;

	double accumulator = 0.0;
	double flickerTime = 0.0;
	Campfire(Entity entity) : pointLight(entity.GetComponent<PointLight>()) {}
	virtual void OnUpdate(double dt) override
	{
		accumulator += dt;

		if (accumulator >= flickerTime)
		{
			accumulator -= flickerTime;
			flickerTime = math::random<double>(0.10, 0.25);

			pointLight.color.r = math::random<float>(0.9f, 1.0f);
			pointLight.color.g = math::random<float>(0.25f, 0.30f);
			pointLight.color.b = math::random<float>(0.0f, 0.05f);
			pointLight.intensity = math::random<float>(7.0f, 10.0f);
		}
	}
};