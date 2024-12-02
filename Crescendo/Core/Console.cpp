#include "console.hpp"

namespace CrescendoEngine
{
	const int desync_io = []() {
		std::ios::sync_with_stdio(false);
		std::cin.tie(nullptr);
		return 0;
	}();

    bool Console::enableThreadSafety = true;
    std::mutex Console::threadMutex;
    Console::severity Console::displayedSeverities =
        static_cast<Console::severity>(
            static_cast<uint8_t>(Console::severity_bits::info) |
            static_cast<uint8_t>(Console::severity_bits::log) |
            static_cast<uint8_t>(Console::severity_bits::warn) |
            static_cast<uint8_t>(Console::severity_bits::error) |
            static_cast<uint8_t>(Console::severity_bits::fatal)
    );
    bool Console::printSeverity = true;
    bool Console::printTimestamp = true;
    std::chrono::time_point<std::chrono::steady_clock> Console::timePoint;
}