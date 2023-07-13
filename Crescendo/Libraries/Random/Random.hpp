#pragma once

#include "RandomEngine.hpp"

namespace Crescendo
{
	class Random
	{
	private:
		static RandomEngine defaultEngine;
	public:
		static void SetSeed(int64_t seed) { defaultEngine.SetSeed(seed); }
		// Returns a random integer between 0 and randMax
		static int64_t Int() { return defaultEngine.Int(); }
		// returns a random integer between min and max inclusive
		static int64_t Int(int64_t min, int64_t max) { return defaultEngine.Int(min, max); }
		// returns a random float between min and max inclusive
		static float Float(float min = 0.0f, float max = 1.0f) { return defaultEngine.Float(min, max); }
		// returns a random double between min and max inclusive
		static double Double(double min = 0.0, double max = 1.0) { return defaultEngine.Double(min, max); }
	};
}