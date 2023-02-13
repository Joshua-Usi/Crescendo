#pragma once

#include <cstdint>
#include <string>

namespace StandardTypes
{
	// Fixed width standard integer types
	typedef std::int8_t int8_t;
	typedef std::uint8_t uint8_t;
	typedef std::int16_t int16_t;
	typedef std::uint16_t uint16_t;
	typedef std::int32_t int32_t;
	typedef std::uint32_t uint32_t;
	typedef std::int64_t int64_t;
	typedef std::uint64_t uint64_t;
}

using namespace StandardTypes;

#ifndef CastVoid
	#define CastVoid(type, data) *static_cast<type*>(data)
#endif

/* only apply to windows */
#if defined(CS_PLATFORM_WINDOWS) && !defined(CS_API)
	#ifdef CS_DYNAMIC_LINKING
		/* this macro is designed to make dll exporting / importing easier */
		#ifdef CS_BUILD_DLL
			#define CS_API __declspec(dllexport)
		#else
			#define CS_API __declspec(dllimport)
		#endif
	#else
		#define CS_API
	#endif
#endif

#ifndef CS_ASSERT
	#if defined(CS_DEBUG) || defined(CS_RELEASE)
		#include <iostream>
		#define CS_ASSERT(x, message)\
			if (!(x))\
			{\
				std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << message << std::endl;\
				abort();\
			}
	#else
		#define CS_ASSERT(x, message)
	#endif
#endif