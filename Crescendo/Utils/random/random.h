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
		void CS_API SetSeed(cs::int64 seed);
		// Returns a random integer between 0 and randMax
		cs::int64 CS_API Int();
		// returns a random integer between min and max inclusive
		cs::int64 CS_API IntBetween(cs::int64 min, cs::int64 max);
		// returns a random float between 0 and 1 inclusive, somewhat similiar to javascripts random
		float CS_API Float();
		// returns a random float between min and max inclusive
		float CS_API FloatBetween(float min, float max);
		// returns a random double between 0 and 1 inclusive, somewhat similiar to javascripts random
		double CS_API Double();
		// returns a random double between min and max inclusive
		double CS_API DoubleBetween(double min, double max);
	}
}