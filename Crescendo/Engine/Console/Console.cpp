#include "Console.hpp"

#include "Core/include/File.hpp"

namespace Crescendo::Engine
{
	void Console::BeginFileLog(const std::string& name)
	{
		logFileName = name;
		isLoggingToFile = true;
		Console::Info("Began logging to file");
	}
	void Console::EndFileLog()
	{
		if (isLoggingToFile)
		{
			// Get localtime as year.month.day and insert into string stream
			std::stringstream ss;
			int64_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			ss << std::put_time(std::localtime(&time), "%Y.%m.%d");

			std::string fileName = std::format("{}-{}.log", logFileName, ss.str());

			Core::TextFile file(fileName);
			if (!file.Exists()) file.Create();
			file.Open().Append(logFileBuffer.str()).AppendLine("==============================================================");

			Console::Info("Saved logs to {}-{}.log", logFileName, ss.str());
			// Reset buffers
			isLoggingToFile = false;
			logFileName.clear();
			logFileBuffer.clear();
		}
		else
		{
			Console::Warn("Ignoring request to finish file logging as logging was never started");
		}
	}
	void Console::SetMinimumSeverity(Severity severity)
	{
		minimumLoggedSeverity = severity;
	}
	void Console::SetThreadSafety(bool enabled)
	{
		enableThreadSafety = enabled;
	}
	void Console::ShowSeverities(bool enabled)
	{
		printSeverity = enabled;
	}
	void Console::ShowTimestamps(bool enabled)
	{
		printTimestamp = enabled;
	}
	void Console::ShowTraces(bool enabled)
	{
		printTrace = enabled;
	}
}