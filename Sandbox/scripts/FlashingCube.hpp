#pragma once
#include "Crescendo.hpp"
using namespace CrescendoEngine;

class FlashingCube : public Behaviour
{
public:
	Material& cubeMaterial;
	Color originalColor;
	float flashSpeed;
	float bloomIntensity;

	FlashingCube(Entity e) : cubeMaterial(e.GetComponent<Material>()) {
		// Make intensity random
		bloomIntensity = math::random(0.5f, 5.0f);
		originalColor = std::get<Color>(cubeMaterial.albedo);
		flashSpeed = math::random(1.0f, 16.0f);
	}
	virtual void OnUpdate(double dt) override
	{
		float intensity = (sin(Application::Get()->GetTime() * flashSpeed) + 0.75f) / 2.0f * bloomIntensity;
		if (intensity < 0.0f) intensity = 0.0f;
		cubeMaterial.emissive = math::vec3(originalColor.r / 255.0f, originalColor.g / 255.0f, originalColor.b / 255.0f) * intensity;
	}
};
