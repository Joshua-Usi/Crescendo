#pragma once

#include "core/core.h"

namespace Crescendo {
	/*
	 *	Uses splitmix algorithm to generate random numbers, insanely fast and efficient,
	 *	good enough to pass Bigcrush but for some seeds it can fail
	 * https://www.pcg-random.org/posts/bugs-in-splitmix.html
	 * 
	 * it also has a 2^64 pattern length
	 */
	/*
	 *	On my laptop (i5-10300H) it generates ints at 650m ops
	 *	Interestingly, generating float and double numbers take the same speed
	 *	And generating a float and double is 5x slower than integers, still 130m ops on
	 *	My laptop, so its not that bad
	 */
	namespace Random {
		void SetSeed(int64_t seed);
		// Returns a random integer between 0 and randMax
		int64_t Int();
		// returns a random integer between min and max inclusive
		int64_t IntBetween(int64_t min, int64_t max);
		// returns a random float between 0 and 1 inclusive, somewhat similiar to javascripts random
		float Float();
		// returns a random float between min and max inclusive
		float FloatBetween(float min, float max);
		// returns a random double between 0 and 1 inclusive, somewhat similiar to javascripts random
		double Double();
		// returns a random double between min and max inclusive
		double DoubleBetween(double min, double max);
	}
}