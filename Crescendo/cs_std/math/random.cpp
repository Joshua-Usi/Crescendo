#include "random.hpp"

namespace cs_std::math
{
	constexpr double RANDOM_MAX = 9223372036854775808.0;

	random_engine random_engine::default_engine = random_engine();

	random_engine::random_engine(int64_t seed)
	{
		this->set_seed(seed);
	}
	void random_engine::set_seed(int64_t seed)
	{
		this->state = seed;
	}
	int64_t random_engine::int64()
	{
		// fractional part of the golden ratio multiplied by 2^64
		this->state += 0x9e3779b97f4a7c15;
		int64_t rand = state;
		rand = (rand ^ (rand >> 30)) * 0xbf58476d1ce4e5b9;
		rand = (rand ^ (rand >> 27)) * 0x94d049bb133111eb;
		return rand ^ (rand >> 31);
	}
	double random_engine::double01()
	{
		return static_cast<double>(this->int64()) / RANDOM_MAX;
	}
	double random_engine::double_range(double low, double high)
	{
		return low + this->double01() * (high - low);
	}

}