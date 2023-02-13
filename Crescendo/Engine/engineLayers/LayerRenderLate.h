#pragma once

#include "core/core.h"
#include "Layer/Layer.h"
#include "Application/Application.h"

namespace Crescendo::Engine {
	// Handles updates and stuff
	class LayerRenderLate : public Layer {
	public:
		using Layer::Layer;

		void OnUpdate(double dt) {
			Application::Get()->GetWindow()->OnLateUpdate();
		}
	};
}
