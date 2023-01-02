#include "console/console.h"

namespace Crescendo::Engine::Console {
	std::mutex threadMutex;

	Severity minimumSeverity = Severity::Info;

	bool printSeverity = true;
	bool printTimestamp = true;
	// TODO CRESCENDO Add traces when C++23 or when non-terminal variadic arguments come out
	bool printTrace = false;
	bool isLoggingToFile = false;
	std::string logFileName = "";
	std::stringstream logFileBuffer;

	void BeginFileLog(std::string name) {
		logFileName = name;
		isLoggingToFile = true;
		Console::EngineInfo("Began logging to file");
	}

	void EndFileLog() {
		if (isLoggingToFile) {
			// Get localtime as year.month.day and insert into string stream
			std::stringstream ss;
			cs::int64 time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			ss << std::put_time(std::localtime(&time), "%y.%m.%d");

			std::string fileName = std::format("{}-{}.log", logFileName, ss.str());

			if (!FileSystem::Exists(fileName)) {
				FileSystem::Create(fileName);
			}

			std::fstream file;
			FileSystem::Open(&file, fileName);
			FileSystem::Write(&file, logFileBuffer.str());
			// Appends 64 = signs so that the end of a program running can be denoted
			FileSystem::Write(&file, "==============================================================\n");
			FileSystem::Close(&file);

			Console::EngineInfo("Saved logs to {}-{}.log", logFileName, ss.str());
			// Reset buffers
			isLoggingToFile = false;
			logFileName.clear();
			logFileBuffer.clear();
		}
		else {
			Console::EngineWarn("Ignoring request to finish file logging as logging was never started");
		}
	}

	void SetMinimumSeverity(Severity severity) {
		minimumSeverity = severity;
	}

	void ShowSeverities(bool enabled) {
		printSeverity = enabled;
	}
	void ShowTimestamps(bool enabled) {
		printTimestamp = enabled;
	}
	void ShowTraces(bool enabled) {
		printTrace = enabled;
	}
}