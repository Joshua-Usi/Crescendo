#include <string>
#include <functional>
#include <iostream>
#include <chrono>

#include "Assert/Assert.h"

namespace Mochi
{
	/// <summary>
	/// Defines a collection of test cases in a class
	/// </summary>
	class Suite
	{
	public:
		Suite(std::string name, std::function<void(Suite*)> func);
		/// <summary>
		/// Add a test case to the suite
		/// </summary>
		/// <param name="caseName">Name or description of the test</param>
		/// <param name="func">Lambda function that contains test contents </param>
		void AddCase(std::string caseName, std::function<bool(void)> func);
		/// <summary>
		/// Runs all test cases
		/// </summary>
		void RunAllTests();
		/// <summary>
		/// Prints the test report detailing passes, fails and which specific ones failed
		/// </summary>
		void PrintReport();
		/// <summary>
		/// Returns report details in JSON form
		/// </summary>
		void GetReportDetails();
	private:
		std::chrono::high_resolution_clock::time_point start;
		std::chrono::high_resolution_clock::time_point end;
		std::string suiteName;
		std::vector<std::function<bool(void)>> cases;
		std::vector<std::string> caseNames;
		std::vector<int> failedTestIDs;
		int casesPassed;
	};
}