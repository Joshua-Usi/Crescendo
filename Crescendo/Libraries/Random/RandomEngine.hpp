#pragma once

#include "Core/common.hpp"

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
	class RandomEngine
	{
	private:
		/*
		 *	The default state is from giving friends a choice of the numbers
		 *	Mitchell - 24
		 *  Tristan 8
		 *	Dom		 - 6755653276514
		 */
		int64_t state;
	private:
		// returns a random float between 0 and 1 inclusive
		float Float01();
		// returns a random double between 0 and 1 inclusive
		double Double01();
	public:
		RandomEngine(int64_t seed = 2486755653276514);
		void SetSeed(int64_t seed);
		// Returns a random integer between 0 and randMax
		int64_t Int();
		// returns a random integer between min and max inclusive
		int64_t Int(int64_t min, int64_t max);
		// returns a random float between min and max inclusive
		float Float(float min = 0.0f, float max = 1.0f);
		// returns a random double between min and max inclusive
		double Double(double min = 0.0, double max = 1.0);
	};
}