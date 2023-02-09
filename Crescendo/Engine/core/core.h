#pragma once

#include "types.h"

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
				std::cout << message << std::endl;\
				abort();\
			}
	#else
		#define CS_ASSERT(x, message)
	#endif
#endif