#pragma once

#include "Crescendo.hpp"

using namespace Crescendo::Engine;
typedef Crescendo::Renderer Renderer;
namespace Graphics = Crescendo::Graphics;
namespace Math = Crescendo::Math;
namespace IO = Crescendo::IO;

class CameraController
{
public:
	unique<Graphics::Camera> camera = {};
	float pMouseX = 0.0f, pMouseY = 0.0f, sens = 0.0005f;

	CameraController() = default;
	inline CameraController(float fov, float aspectRatio, const glm::vec2& clipPlane)
	{
		this->camera.reset(new Graphics::PerspectiveCamera(fov, aspectRatio, clipPlane));
		//this->camera.reset(new Graphics::OrthographicCamera(glm::vec4(-6.4f, 6.4f, -3.6f, 3.6f), clipPlane));
	}
	inline void Update()
	{
		// Camera angles
		double dx = this->pMouseX - Input::GetMouseX(), dy = this->pMouseY - Input::GetMouseY();
		this->pMouseX = Input::GetMouseX(), this->pMouseY = Input::GetMouseY();
		glm::vec3 rotation = camera->GetRotation();
		rotation.y += dx * this->sens;
		rotation.x = std::clamp<float>(rotation.x + dy * this->sens, -Math::PI_2 + 0.01f, Math::PI_2 - 0.01f);
		this->camera->SetRotation(rotation);

		// Camera movement
		float velocity = Input::GetKeyPressed(Key::R) ? 1.0f : 0.1f;
		if (Input::GetKeyPressed(Key::W)) this->camera->MoveAlongForward(velocity);
		if (Input::GetKeyPressed(Key::S)) this->camera->MoveAlongForward(-velocity);
		if (Input::GetKeyPressed(Key::A)) this->camera->MoveAlongRight(-velocity);
		if (Input::GetKeyPressed(Key::D)) this->camera->MoveAlongRight(velocity);

		if (Input::GetKeyPressed(Key::Space)) this->camera->Move(glm::vec3(0.0f, velocity, 0.0f));
		if (Input::GetKeyPressed(Key::ShiftLeft)) this->camera->Move(glm::vec3(0.0f, -velocity, 0.0f));
	}
};