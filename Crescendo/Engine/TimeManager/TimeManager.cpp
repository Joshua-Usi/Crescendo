#include "TimeManager.h"

namespace Crescendo::Engine {
	TimeManager::TimeManager() {
		start = std::chrono::high_resolution_clock::now();
	}
	// TODO CRESCENDO automatically rebase so that precisions of some arbitrary time precision is correct?
	// TODO CRESCENDO auto-rebasing at some arbitrary time
	void TimeManager::Rebase() {
		start = std::chrono::high_resolution_clock::now();
	}
}