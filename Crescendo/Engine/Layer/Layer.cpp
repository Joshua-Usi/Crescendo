#pragma once

#include "Layer.h"

namespace Crescendo::Engine {
	Layer::Layer(double ur, cs::uint64 pr)
	{
		updateRate = ur;
		priority = pr;
		lastCalled = 0.0;
	}

	bool Layer::ShouldRun(double time)
	{
		return time - lastCalled >= updateRate;
	}
}