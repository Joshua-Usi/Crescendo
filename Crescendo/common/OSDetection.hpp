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
#define CS_EXPORT __declspec(dllexport)
#elif defined(CS_TARGET_LINUX) || defined(CS_TARGET_MAC)
#define CS_EXPORT __attribute__((visibility("default")))
#else
#error "Unknown platform"
#endif