#pragma once

#include <cstdint>
#include <string>

// Gauranteed types (gt) for Crescendo. however means some architectures may not be compatible
namespace gt
{
#ifdef USE_C_TYPES
		// C++ standard types
		typedef char Int8;
		typedef unsigned char Uint8;
		typedef short Int16;
		typedef unsigned char Uint16;
		typedef int Int32;
		typedef unsigned int Uint32;
		typedef long long Int64;
		typedef unsigned long long Uint64;

		typedef const char* string;
#else
		// Fixed width integer types
		typedef std::int8_t Int8;
		typedef std::uint8_t Uint8;
		typedef std::int16_t Int16;
		typedef std::uint16_t Uint16;
		typedef std::int32_t Int32;
		typedef std::uint32_t Uint32;
		typedef std::int64_t Int64;
		typedef std::uint64_t Uint64;

		typedef std::string string;
#endif
	typedef std::fstream file;
}