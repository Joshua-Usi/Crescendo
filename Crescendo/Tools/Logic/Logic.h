#pragma once
#include <vector>

namespace Logic
{
	enum class Operator
	{
		None,
		ConstantTrue,
		ConstantFalse,
		Or,
		And,
		Not,
		//Nand,
		//Nor,
		//Xor,
		//Xnor,
	};
	class Gate
	{
	public:
		static Gate True;
		static Gate False;
	private:
		std::vector<Gate*> inputs;
		Operator operation;
	public:
		/// <summary>
		/// Note that this method does not initialise Input values, you must initialise them yourself
		/// </summary>
		/// <param name="oper">Logical operation to undertake</param>
		/// <param name="inputCount">Number of inputs taken</param>
		Gate(Operator oper, unsigned int inputCount = 0);
		void SetInput(unsigned int inputNumber, Gate* state);
		void SetInputBool(unsigned int inputNumber, bool state);
		void SetOperator(Operator op);
		void SetInputCount(unsigned int inputCount);
		bool Output();
	};
}