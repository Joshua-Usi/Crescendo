#pragma once

#include "Crescendo.hpp"
using namespace CrescendoEngine;
#include "cs_std/math/math.hpp"
namespace math = cs_std::math;

#include <numbers>

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

		if (Input::GetKeyPressed(Key::Space)) this->transform->Move(math::vec3(0.0f, velocity, 0.0f));
		// if (Input::GetKeyPressed(Key::ShiftLeft)) this->transform->Move(math::vec3(0.0f, -velocity, 0.0f));

		if (Input::GetKeyDown(Key::L))
		{
			cs_std::console::log(
				"Position:",
				this->transform->position.x,
				this->transform->position.y,
				this->transform->position.z
			);
		}

		// Standard positions for taking photos
		if (Input::GetKeyDown(Key::One))
		{
			this->transform->SetPosition(math::vec3(11.5f, 0.75f, 0.0f));
			this->transform->LookAt(math::vec3(0.0f, 4.0f, 0.0f));
		}
		else if (Input::GetKeyDown(Key::Two))
		{
			this->transform->SetPosition(math::vec3(-11.5f, 0.75f, 0.0f));
			this->transform->LookAt(math::vec3(0.0f, 4.0f, 0.0f));
		}
		else if (Input::GetKeyDown(Key::Three))
		{
			this->transform->SetPosition(math::vec3(-10.0f, 8.0f, -2.0f));
			this->transform->LookAt(math::vec3(0.0f, 4.0f, 0.0f));
		}
		else if (Input::GetKeyDown(Key::Four))
		{
			this->transform->SetPosition(math::vec3(-8.5f, 7.0f, -4.75f));
			this->transform->LookAt(math::vec3(0.0f, 7.0f, -4.75f));
		}
		else if (Input::GetKeyDown(Key::Five))
		{
			this->transform->SetPosition(math::vec3(-8.5f, 2.0f, -4.75f));
			this->transform->LookAt(math::vec3(0.0f, 2.0f, -4.75f));
		}
		else if (Input::GetKeyDown(Key::Six))
		{
			this->transform->SetPosition(math::vec3(-8.5f, 2.25f, 0.0f));
			this->transform->LookAt(math::vec3(-10.0f, 2.25f, 0.0f));
		}
		else if (Input::GetKeyDown(Key::Seven))
		{
			this->transform->SetPosition(math::vec3(8.5f, 2.25f, 0.0f));
			this->transform->LookAt(math::vec3(10.0f, 2.25f, 0.0f));
		}
	}
};