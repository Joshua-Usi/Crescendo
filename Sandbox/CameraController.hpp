#pragma once

#include "Crescendo.hpp"

#include <numbers>

#include "cs_std/math/math.hpp"

using namespace CrescendoEngine;

class CameraController : public Behaviour
{
public:
	PerspectiveCamera camera = {};
	float pMouseX = 0.0f, pMouseY = 0.0f, sens = 0.0005f;

	CameraController() = default;
	CameraController(float fov, float aspectRatio, const cs_std::math::vec2& clipPlane)
	{
		this->camera = PerspectiveCamera(fov, aspectRatio, clipPlane);
	}
	void OnUpdate()
	{
		// Camera angles
		double dx = this->pMouseX - Input::GetMouseX(), dy = this->pMouseY - Input::GetMouseY();
		this->pMouseX = Input::GetMouseX(), this->pMouseY = Input::GetMouseY();
		cs_std::math::vec3 rotation = this->camera.GetRotation();
		rotation.y += dx * this->sens;
		rotation.x = std::clamp<float>(rotation.x + dy * this->sens, -std::numbers::pi / 2 + 0.01f, std::numbers::pi / 2 - 0.01f);
		this->camera.SetRotation(rotation);

		// Camera movement
		float velocity = Input::GetKeyPressed(Key::R) ? 1.0f : 0.1f;
		if (Input::GetKeyPressed(Key::W)) this->camera.MoveAlongForward(velocity);
		if (Input::GetKeyPressed(Key::S)) this->camera.MoveAlongForward(-velocity);
		if (Input::GetKeyPressed(Key::A)) this->camera.MoveAlongRight(-velocity);
		if (Input::GetKeyPressed(Key::D)) this->camera.MoveAlongRight(velocity);

		if (Input::GetKeyPressed(Key::Space)) this->camera.Move(cs_std::math::vec3(0.0f, velocity, 0.0f));
		if (Input::GetKeyPressed(Key::ShiftLeft)) this->camera.Move(cs_std::math::vec3(0.0f, -velocity, 0.0f));
	}
};