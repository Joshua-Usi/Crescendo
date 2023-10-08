	#pragma once

#include "cs_std/console.hpp"

#include <cstdint>
#include <memory>
#include <chrono>

template<typename T>
using unique = std::unique_ptr<T>;
template<typename T>
using shared = std::shared_ptr<T>;

#ifndef CS_ASSERT
	#ifdef CS_DEBUG
		#define CS_ASSERT(x, message)\
			if (!(x))\
			{\
				cs_std::console::fatal(__FILE__, ":", __LINE__, message);\
				abort();\
			}
	#else
		// Still performs the operation as operations can have side effects
		// However, will cause unused expression or did you mean =? warnings
		#define CS_ASSERT(x, message) x
	#endif
#endif

#ifndef CS_ASSERT_ALWAYS
	#define CS_ASSERT_ALWAYS(x, message)\
		if (!(x))\
		{\
			cs_std::console::fatal(__FILE__, ":", __LINE__, message);\
			abort();\
		}
#endif

#ifndef CS_TIME
	// Shows time taken to execute a block of code in milliseconds with 3 decimal places
	#ifdef CS_SHOW_TIMINGS
		#define CS_TIME(code, identifier)\
		{\
			auto start = std::chrono::high_resolution_clock::now();\
			code;\
			std::chrono::duration<double, std::milli> duration = std::chrono::high_resolution_clock::now() - start;\
			cs_std::console::log(identifier, "took", duration.count(), "ms");\
		}
	#else
		#define CS_TIME(code, identifier) code
	#endif
#endif