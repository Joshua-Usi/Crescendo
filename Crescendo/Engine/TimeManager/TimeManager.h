#pragma once

#include <chrono>

#include "core/core.h"

namespace Crescendo::Engine {
	// Manages timing in any arbitrary format
	class TimeManager {
	private:
		std::chrono::high_resolution_clock::time_point start;
	public:
		TimeManager();
		~TimeManager() = default;
		
		/// <summary>
		/// Returns the time since this TimeManager was initialised, in seconds
		/// </summary>
		/// <typeparam name="return_type">A basic C type to return</typeparam>
		/// <returns>Time in seconds in the C type</returns>
		template<typename return_type>
		return_type GetTimeSinceStart() {
			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<return_type> duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start);
			return duration.count();
		}
		/// <summary>
		/// Set the current time to now, used to alleviate floating point errors that can arise from using the program for a very long time or high-precision timing
		/// https://stackoverflow.com/a/41210880/11098922
		/// </summary>
		void Rebase();
	};
}

