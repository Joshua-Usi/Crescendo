#pragma once

#include "Crescendo.hpp"
using namespace CrescendoEngine;
#include <numbers>
#include "cs_std/math/math.hpp"

class CameraController : public Behaviour
{
public:
	Transform* transform;
	float pMouseX = 0.0f, pMouseY = 0.0f, sens = 0.0005f;

	CameraController() = default;

	virtual void OnAttach(Entity e) override
	{
		this->transform = &e.GetComponent<Transform>();
	}
	virtual void OnUpdate(double dt) override
	{
		// Camera angles
		double dx = this->pMouseX - Input::GetMouseX(), dy = this->pMouseY - Input::GetMouseY();
		this->pMouseX = Input::GetMouseX(), this->pMouseY = Input::GetMouseY();
		cs_std::math::vec3 rotation = this->transform->GetRotation();
		rotation.y += dx * this->sens;
		rotation.x = std::clamp<float>(rotation.x + dy * this->sens, -std::numbers::pi / 2 + 0.01f, std::numbers::pi / 2 - 0.01f);
		this->transform->SetRotation(rotation);

		// Camera movement
		float velocity = Input::GetKeyPressed(Key::R) ? 1.0f : 0.1f;
		if (Input::GetKeyPressed(Key::W)) this->transform->MoveAlongForward(velocity);
		if (Input::GetKeyPressed(Key::S)) this->transform->MoveAlongForward(-velocity);
		if (Input::GetKeyPressed(Key::A)) this->transform->MoveAlongRight(-velocity);
		if (Input::GetKeyPressed(Key::D)) this->transform->MoveAlongRight(velocity);

		if (Input::GetKeyPressed(Key::Space)) this->transform->Move(cs_std::math::vec3(0.0f, velocity, 0.0f));
		if (Input::GetKeyPressed(Key::ShiftLeft)) this->transform->Move(cs_std::math::vec3(0.0f, -velocity, 0.0f));
	}
};