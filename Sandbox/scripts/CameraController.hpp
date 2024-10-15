#pragma once
#include "Crescendo.hpp"
using namespace CrescendoEngine;

#include <numbers>

class CameraController : public Behaviour
{
public:
	Transform& transform;
	float pMouseX = 0.0f, pMouseY = 0.0f, sens = 0.0005f;

	CameraController(Entity e) : transform(e.GetComponent<Transform>()) {}
	virtual void OnUpdate(double dt) override
	{
		// Get the input system of the first window
		Input* input = Application::Get()->GetWindow(0)->GetInput();

		// Camera angles
		double dx = pMouseX - input->GetMouseX(), dy = pMouseY - input->GetMouseY();
		pMouseX = input->GetMouseX(), pMouseY = input->GetMouseY();
		cs_std::math::vec3 rotation = transform.GetRotation();
		rotation.y += dx * sens;
		rotation.x = std::clamp<float>(rotation.x + dy * sens, -std::numbers::pi / 2 + 0.01f, std::numbers::pi / 2 - 0.01f);
		transform.SetRotation(rotation);

		// Camera movement
		float velocity = input->GetKeyPressed(Key::R) ? 1.0f : 0.1f;
		if (input->GetKeyPressed(Key::W)) transform.MoveAlongForward(velocity);
		if (input->GetKeyPressed(Key::S)) transform.MoveAlongForward(-velocity);
		if (input->GetKeyPressed(Key::A)) transform.MoveAlongRight(-velocity);
		if (input->GetKeyPressed(Key::D)) transform.MoveAlongRight(velocity);

		if (input->GetKeyPressed(Key::Space)) transform.Move(math::vec3(0.0f, velocity, 0.0f));
		if (input->GetKeyPressed(Key::ShiftLeft)) transform.Move(math::vec3(0.0f, -velocity, 0.0f));

		if (input->GetKeyDown(Key::L))
		{
			cs_std::console::log(
				"Position:",
				transform.position.x,
				transform.position.y,
				transform.position.z
			);
		}

		// Standard positions for taking photos
		if (input->GetKeyDown(Key::One))
		{
			transform.SetPosition(math::vec3(11.5f, 0.75f, 0.0f));
			transform.LookAt(math::vec3(0.0f, 4.0f, 0.0f));
		}
		else if (input->GetKeyDown(Key::Two))
		{
			transform.SetPosition(math::vec3(-11.5f, 0.75f, 0.0f));
			transform.LookAt(math::vec3(0.0f, 4.0f, 0.0f));
		}
		else if (input->GetKeyDown(Key::Three))
		{
			transform.SetPosition(math::vec3(-10.0f, 8.0f, -2.0f));
			transform.LookAt(math::vec3(0.0f, 4.0f, 0.0f));
		}
		else if (input->GetKeyDown(Key::Four))
		{
			transform.SetPosition(math::vec3(-8.5f, 7.0f, -4.75f));
			transform.LookAt(math::vec3(0.0f, 7.0f, -4.75f));
		}
		else if (input->GetKeyDown(Key::Five))
		{
			transform.SetPosition(math::vec3(-8.5f, 2.0f, -4.75f));
			transform.LookAt(math::vec3(0.0f, 2.0f, -4.75f));
		}
		else if (input->GetKeyDown(Key::Six))
		{
			transform.SetPosition(math::vec3(-8.5f, 2.25f, 0.0f));
			transform.LookAt(math::vec3(-10.0f, 2.25f, 0.0f));
		}
		else if (input->GetKeyDown(Key::Seven))
		{
			transform.SetPosition(math::vec3(8.5f, 2.25f, 0.0f));
			transform.LookAt(math::vec3(10.0f, 2.25f, 0.0f));
		}
	}
};