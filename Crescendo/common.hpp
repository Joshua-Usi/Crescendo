#pragma once

#include "cs_std/console.hpp"

// commonly used includes
#include <cstdint>
#include <memory>
#include <vector>
#include <array>
#include <string>

#define CS_NAMESPACE_BEGIN namespace CrescendoEngine

#define CS_DEFINE_ENUM_CLASS_OR_OPERATOR(EnumName)\
	inline EnumName operator|(EnumName a, EnumName b) { return static_cast<EnumName>(static_cast<std::underlying_type_t<EnumName>>(a) | static_cast<std::underlying_type_t<EnumName>>(b)); };\
	inline EnumName operator|=(EnumName& a, EnumName b) { return a = a | b; }
#define CS_DEFINE_ENUM_CLASS_AND_OPERATOR(EnumName)\
	inline EnumName operator&(EnumName a, EnumName b) { return static_cast<EnumName>(static_cast<std::underlying_type_t<EnumName>>(a) & static_cast<std::underlying_type_t<EnumName>>(b)); };\
	inline EnumName operator&=(EnumName& a, EnumName b) { return a = a & b; }
#define CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(EnumName) CS_DEFINE_ENUM_CLASS_OR_OPERATOR(EnumName); CS_DEFINE_ENUM_CLASS_AND_OPERATOR(EnumName)

#ifndef CS_DEBUG_ASSERT
	#ifdef CS_DEBUG
		// Only active in debug builds
		#define CS_DEBUG_ASSERT(x, message) if (!(x)) { cs_std::console::fatal(__FILE__, ":", __LINE__, ' ', message); abort(); }
	#else
		#define CS_DEBUG(x, message) x
	#endif
#endif

#ifndef CS_ASSERT
	// Active asserts that must be satisfied otherwise the program will terminate. Use for places where errors in data are not recoverable
	#define CS_ASSERT(x, message) if (!(x)) { cs_std::console::fatal(__FILE__, ":", __LINE__, ' ', message); abort(); }
#endif

#ifndef CS_ASSERT_WARNING
	// Assertive warnings without stopping execution, it can denote invalid data but not necessarily a fatal error
	#define CS_ASSERT_WARNING(x, message) if (!(x)) { cs_std::console::warn(__FILE__, ":", __LINE__, ' ', message); }
#endif
