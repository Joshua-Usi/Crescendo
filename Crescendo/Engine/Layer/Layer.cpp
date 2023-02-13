#pragma once

#include "Layer.h"

namespace Crescendo::Engine {
	Layer::Layer(double ur, uint64_t pr)
	{
		this->updateRate = ur;
		this->priority = pr;
		this->lastCalled = 0.0;
	}

	bool Layer::ShouldRun(double time)
	{
		return time - this->lastCalled >= this->updateRate;
	}
}