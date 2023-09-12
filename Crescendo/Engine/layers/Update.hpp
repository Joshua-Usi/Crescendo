#pragma once

#include "Core/include/Layer.hpp"
#include "Engine/Application/Application.hpp"
#include "Engine/interfaces/Input.hpp"

namespace Crescendo::Engine
{
	// Handles updates and stuff
	class LayerUpdate : public Layer
	{
	public:
		LayerUpdate(double a, int b) : Layer(a, b) {}
		inline virtual void OnUpdate(double dt) override final
		{
			Input::PollEvents();
			Application::Get()->OnUpdate(dt);
		}
	};
}
