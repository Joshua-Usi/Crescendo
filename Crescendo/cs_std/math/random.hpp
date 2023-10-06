#pragma once

#include <cstdint>
#include <limits>

namespace cs_std::math
{
	class random_engine
	{
	private:
		int64_t state;
	public:
		static random_engine default_engine;
		random_engine(int64_t seed = 2486755653276514);
		void set_seed(int64_t seed);

		int64_t int64();
		double double01();
		double double_range(double low, double high);
	};

	template<typename T = int64_t>
	T random(T low = 0.0, T high = 0.0)
	{
		return static_cast<T>(random_engine::default_engine.double_range(static_cast<double>(low), static_cast<double>(high) + 1.0f));
	}
}