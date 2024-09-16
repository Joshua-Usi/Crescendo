#pragma once
#include "Layers/Layer.hpp"
#include "Core/Application.hpp"
#include "Interfaces/Input.hpp"

CS_NAMESPACE_BEGIN
{
	// Handles updates and stuff
	class LayerUpdate : public Layer
	{
	public:
		LayerUpdate(double a) : Layer(a) {}
		virtual void OnUpdate(double dt) override final
		{
			Application* app = Application::Get();
			for (size_t i = 0, cnt = app->GetWindowCount(); i < cnt; i++)
			{
				app->GetWindow(i)->GetInput()->PollEvents();
			}
			Application::Get()->InternalUpdate(dt);
			Application::Get()->OnUpdate(dt);
		}
	};
}
