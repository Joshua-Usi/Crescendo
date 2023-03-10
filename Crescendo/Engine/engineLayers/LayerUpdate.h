#pragma once

#include "core/core.h"
#include "Layer/Layer.h"
#include "Application/Application.h"

namespace Crescendo::Engine {
	// Handles updates and stuff
	class LayerUpdate : public Layer {
	public:
		using Layer::Layer;

		void OnAttach() {}
		void OnDetach() {}
		void OnInit() {}
		void OnUpdate(double dt) {
			Application::Get()->OnUpdate(dt);
		}
	};
}
