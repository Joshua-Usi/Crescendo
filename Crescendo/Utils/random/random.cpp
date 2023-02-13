#include "random.h"

// 2 ** 63
#define CS_RAND_MAX	9223372036854775808

namespace Crescendo {
	namespace Random {
		/*
		 *	The default state is from giving friends a choice of the numbers
		 *	Mitchell - 24
		 *  Tristan 8
		 *	Dom		 - 6755653276514
		 */
		int64_t state = 2486755653276514;
		void SetSeed(int64_t seed) {
			state = seed;
		}
		int64_t Int() {
			// fractional part of the golden ratio multiplied by 2^64 (i think)
			state += 0x9e3779b97f4a7c15;
			int64_t rand = state;
			rand = (rand ^ (rand >> 30)) * 0xbf58476d1ce4e5b9;
			rand = (rand ^ (rand >> 27)) * 0x94d049bb133111eb;
			return rand ^ (rand >> 31);
		}
		int64_t IntBetween(int64_t min, int64_t max) {
			// converting to doubles is faster than modulus
			//return  min + Int() % (max - min);
			return int64_t(DoubleBetween(float(min), float(max) + 1));
		}
		float Float() {
			return float(Int()) / CS_RAND_MAX;
		}
		float FloatBetween(float min, float max) {
			return min + Float() * (max - min);
		}
		double Double() {
			return double(Int()) / CS_RAND_MAX;
		}
		double DoubleBetween(double min, double max) {
			return min + Double() * (max - min);
		}
	}
}