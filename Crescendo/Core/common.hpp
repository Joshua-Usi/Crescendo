#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <iostream>
#include <chrono>

template<typename type>
using unique = std::unique_ptr<type>;
template<typename type>
using shared = std::shared_ptr<type>;

#ifndef CS_ASSERT
	#ifdef CS_DEBUG
		#define CS_ASSERT(x, message)\
			if (!(x))\
			{\
				std::cout << "[" << __FILE__ << ":" << __LINE__ << "] " << message << std::endl;\
				abort();\
			}
	#else
		// Still performs the operation as operations can have side effects
		// However, will cause unused expression or did you mean =? warnings
		#define CS_ASSERT(x, message) x
	#endif
#endif

#define CS_SHOW_TIMINGS

#ifndef CS_TIME
	// Shows time taken to execute a block of code in milliseconds with 3 decimal places
	#ifdef CS_SHOW_TIMINGS
		#define CS_TIME(code, identifier)\
		{\
			auto start = std::chrono::high_resolution_clock::now();\
			code;\
			std::chrono::duration<double, std::milli> duration = std::chrono::high_resolution_clock::now() - start;\
			std::cout << identifier << " took " << duration.count() << "ms" << std::endl;\
		}
	#else
		#define CS_TIME(code, identifier) code
	#endif
#endif