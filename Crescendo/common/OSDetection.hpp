#pragma once

// Detects the target platform
#if defined(_WIN32) || defined(_WIN64)
	#define CS_TARGET_WINDOWS
#elif defined(__linux__)
	#define CS_TARGET_LINUX
	#error "Linux is not supported yet."
#elif defined(__APPLE__) && defined(__MACH__)
	#define CS_TARGET_MAC
	#error "macOS is not supported yet."
#else
	#error "Unsupported platform. This macro only supports Windows, Linux, and macOS."
#endif

// Exporting symbols
#if defined(CS_TARGET_WINDOWS)
	// Normal exports
	#ifdef CS_BUILDING_MODULE_DLL
		#define CS_MODULE_EXPORT __declspec(dllexport)
	#else
		#define CS_MODULE_EXPORT __declspec(dllimport)
	#endif
	// Exports specifically for building the common DLL
	#ifdef CS_BUILDING_COMMON_DLL
		#define CS_COMMON_EXPORT __declspec(dllexport)
	#else
		#define CS_COMMON_EXPORT __declspec(dllimport)
	#endif
	// Exports specifically for building the core DLL
	#ifdef CS_BUILDING_CORE
		#define CS_CORE_EXPORT
	#else
		#define CS_CORE_EXPORT __declspec(dllimport)
	#endif
#elif defined(CS_TARGET_LINUX) || defined(CS_TARGET_MAC)
	#define CS_EXPORT __attribute__((visibility("default")))
#else
	#error "Unknown platform"
#endif