#include "Mochi/Mochi.h"

int main()
{
	Mochi::Suite meta("Mochi Meta-tests", [&](Mochi::Suite* suite) {
		suite->AddCase("Asserts if 2 values are equal", [&]() {
			return Mochi::Assert::That(3).Is.EqualTo(3).AssertsTrue(); // True
		});
		suite->AddCase("Asserts if any numerical datatypes are equal", [&]() {
			return Mochi::Assert::That(2.5f).Is.EqualTo(2.5f).AssertsTrue(); // True
		});
		suite->AddCase("Asserts if 2 values not equal using NOT", [&]() {
			return Mochi::Assert::That(3).Is.Not().EqualTo(5).AssertsTrue(); // True
		});
		suite->AddCase("Asserts if a value is less than another", [&]() {
			return Mochi::Assert::That(6.4f).Is.LessThan(9.3f).AssertsTrue(); // True
		});
		suite->AddCase("Asserts if a value is greater than another", [&]() {
			return Mochi::Assert::That(13.8f).Is.GreaterThan(9.3f).AssertsTrue(); // True
		});
		suite->AddCase("Multiple asserts if a value is less than or equal to another using OR", [&]() {
			return Mochi::Assert::That(4).Is.LessThan(6).Or().EqualTo(6).AssertsTrue(); // True
		});
		suite->AddCase("Multiple asserts if a value is within a certain range using AND", [&]() {
			return Mochi::Assert::That(6).Is.GreaterThan(5).And().LessThan(7).AssertsTrue();
		});
		suite->AddCase("Multiple asserts combining several operations", [&]() {
			return Mochi::Assert::That(7).Is.Not().EqualTo(5).Or().LessThan(9).AssertsTrue();
		});
		suite->AddCase("Asserts booleans", [&]() {
			return Mochi::Assert::That(true).Is.True().AssertsTrue();
		});
		suite->AddCase("Asserts if pointers point to null", [&]() {
			return Mochi::Assert::That(NULL).Is.Null().AssertsTrue();
		});
		suite->AddCase("Asserts if 2 pointer regions contain exactly the same bytes", [&]() {
			int numbers[5] { 4, 6, 13, 7, 9 };
			int numbers2[5]{ 4, 6, 13, 7, 9 };
			return Mochi::Assert::That(&numbers).Equals(&numbers2).AssertsTrue();
		});
		suite->AddCase("Asserts if 2 pointer regions are not the same", [&]() {
			int numbers[5]{ 4, 6, 13, 7, 9 };
			int numbers2[5]{ 4, 2, 11, 7, 9 };
			return Mochi::Assert::That(&numbers).Not().Equals(&numbers2).AssertsTrue();
		});
		suite->AddCase("Long Running Tests", [&]() {
			long long sum = 0;
			long long n = 100000000;
			for (int i = 0; i <= n; i++)
			{
				sum += i;
			}
			return Mochi::Assert::That(sum).Is.EqualTo(n * (n + 1) / 2).AssertsTrue();
		});
	});

	Mochi::Suite meta_logic("Tests the logic behind Mochi tests", [&](Mochi::Suite* suite) {
		suite->AddCase("And gate logic: Equivalent to 'true && true'", [&]() {
			Mochi::Logic::Gate gate(Mochi::Logic::Operator::And, 2);
			gate.SetInputBool(0, true);
			gate.SetInputBool(1, true);
			return Mochi::Assert::That(gate.Output()).Is.True().AssertsTrue();
		});
		suite->AddCase("And gate logic: Equivalent to 'true && false'", [&]() {
			Mochi::Logic::Gate gate(Mochi::Logic::Operator::And, 2);
			gate.SetInputBool(0, true);
			gate.SetInputBool(1, false);
			return Mochi::Assert::That(gate.Output()).Is.Not().True().AssertsTrue();
		});
		suite->AddCase("Multi-Input And gate logic: Equivalent to 'true && true && true && true && true'", [&]() {
			Mochi::Logic::Gate gate(Mochi::Logic::Operator::And, 5);
			gate.SetInputBool(0, true);
			gate.SetInputBool(1, true);
			gate.SetInputBool(2, true);
			gate.SetInputBool(3, true);
			gate.SetInputBool(4, true);
			return Mochi::Assert::That(gate.Output()).Is.True().AssertsTrue();
		});
		suite->AddCase("Or gate logic: Equivalent to 'false || true'", [&]() {
			Mochi::Logic::Gate gate(Mochi::Logic::Operator::Or, 2);
			gate.SetInputBool(0, false);
			gate.SetInputBool(1, true);
			return Mochi::Assert::That(gate.Output()).Is.True().AssertsTrue();
		});
		suite->AddCase("Multi-Input Or gate logic: Equivalent to 'false || false || true || false || false'", [&]() {
			Mochi::Logic::Gate gate(Mochi::Logic::Operator::Or, 5);
			gate.SetInputBool(0, false);
			gate.SetInputBool(1, false);
			gate.SetInputBool(2, true);
			gate.SetInputBool(3, false);
			gate.SetInputBool(4, false);
			return Mochi::Assert::That(gate.Output()).Is.True().AssertsTrue();
		});
		suite->AddCase("Not gate logic: Equivalent to '!true'", [&]() {
			Mochi::Logic::Gate gate(Mochi::Logic::Operator::Not, 1);
			gate.SetInputBool(0, true);
			return Mochi::Assert::That(gate.Output()).Is.Not().True().AssertsTrue();
		});
		suite->AddCase("Nested Gate Logic: Equivalent to '(true && true) && (true && false)'", [&]() {
			Mochi::Logic::Gate gate(Mochi::Logic::Operator::And, 2);

			Mochi::Logic::Gate gateSub1(Mochi::Logic::Operator::And, 2);
			gateSub1.SetInputBool(0, true);
			gateSub1.SetInputBool(1, true);

			Mochi::Logic::Gate gateSub2(Mochi::Logic::Operator::And, 2);
			gateSub2.SetInputBool(0, true);
			gateSub2.SetInputBool(1, false);

			gate.SetInput(0, &gateSub1);
			gate.SetInput(1, &gateSub2);
			return Mochi::Assert::That(gate.Output()).Is.Not().True().AssertsTrue();
		});

	});

	meta.RunAllTests();
	meta_logic.RunAllTests();

	meta.PrintReport();
	meta_logic.PrintReport();
	return 0;
}