#pragma once
#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	class Layer
	{
	public:
		double updateRate, lastCalled; // In seconds
	public:
		Layer(double ur) : updateRate(ur), lastCalled(0.0) {};
		bool ShouldRun(double time) const { return time - this->lastCalled >= this->updateRate; }
		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnInit() {};
		virtual void OnUpdate(double dt) {};
	};
}