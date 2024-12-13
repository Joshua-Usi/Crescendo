#include "Console.hpp"

namespace CrescendoEngine
{
	const int desync_io = []() {
		std::ios::sync_with_stdio(false);
		std::cin.tie(nullptr);
		return 0;
	}();

	CS_CORE_EXPORT bool Console::enableThreadSafety = true;
	CS_CORE_EXPORT std::mutex Console::threadMutex;
	CS_CORE_EXPORT Console::severity Console::displayedSeverities =
        static_cast<Console::severity>(
            static_cast<uint8_t>(Console::severity_bits::info) |
            static_cast<uint8_t>(Console::severity_bits::log) |
            static_cast<uint8_t>(Console::severity_bits::warn) |
            static_cast<uint8_t>(Console::severity_bits::error) |
            static_cast<uint8_t>(Console::severity_bits::fatal)
    );
	CS_CORE_EXPORT bool Console::printSeverity = true;
	CS_CORE_EXPORT bool Console::printTimestamp = true;
	CS_CORE_EXPORT std::chrono::time_point<std::chrono::steady_clock> Console::timePoint;

	void Console::SetSeverityFlags(severity severityFlag)
	{
		displayedSeverities = severityFlag;
	}
	void Console::SetThreadSafety(bool enable)
	{
		enableThreadSafety = enable;
	}
	void Console::SetTimestampPrinting(bool enable)
	{
		printTimestamp = enable;
	}
	void Console::SetSeverityPrinting(bool enable)
	{
		printSeverity = enable;
	}
	void Console::Begin()
	{
		timePoint = std::chrono::high_resolution_clock::now();
	}
}