#pragma once

namespace cs_std::math
{
	template<typename T>
	T map(T value, T min, T max, T newMin, T newMax)
	{
		return (value - min) / (max - min) * (newMax - newMin) + newMin;
	}
}