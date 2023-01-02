#include "Logic.h"

namespace Logic
{
	Gate::Gate(Operator oper, unsigned int inputCount)
	{
		inputs.resize(inputCount);
		operation = oper;
	}
	void Gate::SetInput(unsigned int inputNumber, Gate* state)
	{
		inputs[inputNumber] = state;
	}
	void Gate::SetInputBool(unsigned int inputNumber, bool state)
	{
		inputs[inputNumber] = (state) ? &True : &False;
	}
	void Gate::SetOperator(Operator op)
	{
		operation = op;
	}
	void Gate::SetInputCount(unsigned int inputCount)
	{
		inputs.resize(inputCount);
	}
	bool Gate::Output()
	{
		switch (operation)
		{
		case Operator::ConstantTrue: return true;
		case Operator::ConstantFalse: return false;
		case Operator::Or:
		{
			bool result = inputs[0]->Output();
			for (int i = 1; i < inputs.size(); i++)
			{
				result |= inputs[i]->Output();
			}
			return result;
		};
		case Operator::And:
		{
			bool result = inputs[0]->Output();
			for (int i = 1; i < inputs.size(); i++)
			{
				result &= inputs[i]->Output();
			}
			return result;
		};
		case Operator::Not: return !inputs[0]->Output();
		}
		return true;
	};

	Gate Gate::False(Operator::ConstantFalse);
	Gate Gate::True(Operator::ConstantTrue);
}