#include "RandomEngine.hpp"

#include "Core/common.hpp"

// 2 ** 63
#define CS_RAND_MAX	9223372036854775808

namespace Crescendo:: Math
{
	RandomEngine::RandomEngine(int64_t seed)
	{
		this->SetSeed(seed);
	}
	void RandomEngine::SetSeed(int64_t seed) {
		this->state = seed;
	}
	int64_t RandomEngine::Int() {
		// fractional part of the golden ratio multiplied by 2^64
		this->state += 0x9e3779b97f4a7c15;
		int64_t rand = state;
		rand = (rand ^ (rand >> 30)) * 0xbf58476d1ce4e5b9;
		rand = (rand ^ (rand >> 27)) * 0x94d049bb133111eb;
		return rand ^ (rand >> 31);
	}
	int64_t RandomEngine::Int(int64_t min, int64_t max) {
		/*	converting to doubles is faster than modulus
		 *	And the performance difference between floats and doubles is negligible
		 *	+1.0f since truncation rounds down to 0
		 */
		return int64_t(this->Double(double(min), double(max) + 1.0f));
	}
	float RandomEngine::Float01() {
		return float(this->Int()) / CS_RAND_MAX;
	}
	float RandomEngine::Float(float min, float max) {
		return min + this->Float01() * (max - min);
	}
	double RandomEngine::Double01() {
		return double(this->Int()) / CS_RAND_MAX;
	}
	double RandomEngine::Double(double min, double max) {
		return min + this->Double01() * (max - min);
	}
}