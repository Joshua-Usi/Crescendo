#pragma once

#include "cs_std/console.hpp"

// commonly used includes
#include <cstdint>
#include <memory>
#include <vector>
#include <array>
#include <string>

#define CS_NAMESPACE_BEGIN namespace CrescendoEngine

#ifndef CS_ASSERT
	#ifdef CS_DEBUG
		#define CS_ASSERT(x, message) if (!(x)) { cs_std::console::fatal(__FILE__, ":", __LINE__, message); abort(); }
	#else
		#define CS_ASSERT(x, message) x
	#endif
#endif