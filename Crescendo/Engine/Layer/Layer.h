#pragma once

#include "core/core.h"

namespace Crescendo::Engine
{
	class CS_API Layer
	{
	public:
		Layer(double ur, cs::uint64 pr);
		// 0 means as fast as possible, measured in seconds
		double updateRate;
		double lastCalled;
		cs::uint64 priority;

		bool ShouldRun(double time);

		/// <summary>
		/// Runs once when layers are attached
		/// </summary>
		virtual void OnAttach()
		{};
		/// <summary>
		/// Runs once when layers are detached
		/// </summary>
		virtual void OnDetach()
		{};
		/// <summary>
		/// Run once when all layers are attached. Initialises in order
		/// </summary>
		virtual void OnInit()
		{};
		/// <summary>
		/// Run every time time - lastCalled >= updateRate;
		/// </summary>
		/// <param name="dt">delta time between the previous and current step</param>
		virtual void OnUpdate(double dt)
		{};
	};
}