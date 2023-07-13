#pragma once

#include "Core/include/Layer.hpp"
#include "Engine/Application/Application.hpp"

#include "GLFW/glfw3.h"

namespace Crescendo::Core {
	// Handles updates and stuff
	class LayerUpdate : public Layer {
	public:
		LayerUpdate(double a, int b) : Layer(a, b) {}
		inline virtual void OnUpdate(double dt) override final {
			Engine::Application::Get()->OnUpdate(dt);
			glfwPollEvents();
		}
	};
}
