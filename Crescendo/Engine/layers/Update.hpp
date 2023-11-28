#pragma once

#include "Engine/LayerStack/Layer.hpp"
#include "Engine/Application/Application.hpp"
#include "Engine/interfaces/Input.hpp"

CS_NAMESPACE_BEGIN
{
	// Handles updates and stuff
	class LayerUpdate : public Layer
	{
	public:
		LayerUpdate(double a) : Layer(a) {}
		virtual void OnUpdate(double dt) override final
		{
			Input::PollEvents();
			Application::Get()->OnUpdate(dt);
		}
	};
}
