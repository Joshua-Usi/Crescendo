#pragma once
#include <cstring>
#include <cmath>

#include "Logic/Logic.h"

namespace Assert
{
	template <typename type>
	class That
	{
	private:
		// One input AND gates act like repeaters, propagating signals through
		Logic::Gate* satisfaction = new Logic::Gate(Logic::Operator::And, 1);
		type state;
		int chainSize;
	public:
		That* Is = this;
		That(type value)
		{
			state = value;
			chainSize = 0;
		}
		~That()
		{
			delete satisfaction;
		}
		/// <summary>
		/// Logical NOT gate that inverts the next operation to return the opposite boolean value
		/// </summary>
		That* Not()
		{
			Logic::Gate* inverter = new Logic::Gate(Logic::Operator::Not, 1);
			inverter->SetInput(chainSize, satisfaction);
			satisfaction = inverter;
			return this;
		}
		/// <summary>
		/// Logical OR gate that returns true if one or more inputs is true
		/// </summary>
		That* Or()
		{
			satisfaction->SetOperator(Logic::Operator::Or);
			return this;
		}
		/// <summary>
		/// Logical AND gate that only returns true if all inputs are true
		/// </summary>
		/// <param name="value">The values to compare with</param>
		That* And()
		{
			satisfaction->SetOperator(Logic::Operator::And);
			return this;
		}
		/// <summary>
		/// Comparator that returns true if the given value is equal to the asserted value
		/// </summary>
		/// <param name="value">Value to compare</param>
		That* EqualTo(type value)
		{
			satisfaction->SetInputCount(chainSize + 1);
			satisfaction->SetInputBool(chainSize, state == value);
			chainSize++;
			return this;
		}
		/// <summary>
		/// Comparator that returns true if the given value is less than the asserted value
		/// </summary>
		/// <param name="value">Value to compare</param>
		That* LessThan(type value)
		{
			satisfaction->SetInputCount(chainSize + 1);
			satisfaction->SetInputBool(chainSize, state < value);
			chainSize++;
			return this;
		}
		/// <summary>
		/// Comparator that returns true if the given value is greater than the asserted value
		/// </summary>
		/// <param name="value">Value to compare</param>
		That* GreaterThan(type value)
		{
			satisfaction->SetInputCount(chainSize + 1);
			satisfaction->SetInputBool(chainSize, state > value);
			chainSize++;
			return this;
		}
		/// <summary>
		/// Floating point comparator that returns true if the asserted value is NaN
		/// </summary>
		That* NaN()
		{
			satisfaction->SetInputCount(chainSize + 1);
			satisfaction->SetInputBool(chainSize, std::isnan(state));
			chainSize++;
			return this;
		}
		/// <summary>
		/// Floating point comparator that returns true if the asserted value is Finite
		/// </summary>
		That* Finite()
		{
			satisfaction->SetInputCount(chainSize + 1);
			satisfaction->SetInputBool(chainSize, !std::isinf(state));
			chainSize++;
			return this;
		}
		/// <summary>
		/// Boolean comparator that returns true if the asserted value is true or truthy
		/// </summary>
		That* True()
		{
			satisfaction->SetInputCount(chainSize + 1);
			satisfaction->SetInputBool(chainSize, state);
			chainSize++;
			return this;
		}
		/// <summary>
		/// Memory comparator that returns true if the 2 memory locations are exactly equal bit for bit
		/// </summary>
		/// <param name="memregion">Starting address of memory region to compare</param>
		That* Equals(type memregion)
		{
			int size = sizeof(type);
			satisfaction->SetInputCount(chainSize + 1);
			satisfaction->SetInputBool(chainSize, memcmp(state, memregion, size) == 0);
			chainSize++;
			return this;
		}
		/// <summary>
		/// Memory comparator that returns true if the memory address points to a Null Address (0x0)
		/// </summary>
		That* Null()
		{
			satisfaction->SetInputCount(chainSize + 1);
			satisfaction->SetInputBool(chainSize, state == 0);
			chainSize++;
			return this;
		}
		bool AssertsTrue()
		{
			return satisfaction->Output();
		}
	};
}