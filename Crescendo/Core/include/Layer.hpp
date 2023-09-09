#pragma once

#include "Core/common.hpp"

namespace Crescendo
{
	class Layer
	{
	public:
		// 0 means as fast as possible, measured in seconds
		double updateRate, lastCalled;
		uint64_t priority;
	public:
		Layer(double ur, uint64_t pr) : updateRate(ur), priority(pr), lastCalled(0.0) {};
		virtual bool ShouldRun(double time) const final { return time - this->lastCalled >= this->updateRate; }
		/// <summary>
		/// Runs once when layers are attached
		/// </summary>
		virtual void OnAttach() {};
		/// <summary>
		/// Runs once when layers are detached
		/// </summary>
		virtual void OnDetach() {};
		/// <summary>
		/// Run once when all layers are attached. Initialises in order
		/// </summary>
		virtual void OnInit() {};
		/// <summary>
		/// Run every time time - lastCalled >= updateRate;
		/// </summary>
		/// <param name="dt">delta time between the previous and current step</param>
		virtual void OnUpdate(double dt) {};
	};
}