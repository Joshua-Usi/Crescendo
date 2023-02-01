#pragma once

#include <chrono>
#include <iostream>
#include <mutex>
#include <format>

#include "core/core.h"
#include "filesystem/synchronous/syncFiles.h"

/* I love this macro */
#ifndef CS_PROD
#define CS_DEFINE_LOGGER(name, severity)\
	template<typename... Args>\
	void name(std::string message, Args... args)\
	{\
		if (minimumSeverity <= severity) {\
			_UnclassifiedLog("", "["#name"] ", message, args...);\
		}\
	}\
	template<typename... Args>\
	void Engine##name(std::string message, Args... args)\
	{\
		if (minimumSeverity <= severity) {\
			_UnclassifiedLog("(Crescendo) ", "["#name"] ", message, args...);\
		}\
	}
#else
#define CS_DEFINE_LOGGER(name, severity)\
	template<typename... Args>\
	void name(std::string message, Args... args) {}\
	template<typename... Args>\
	void Engine##name(std::string message, Args... args) {}
#endif

namespace Crescendo::Engine::Console
{
	enum class Severity { Debug, Info, Log, Warn, Error, Critical };

	extern CS_API std::mutex threadMutex;

	extern CS_API Severity minimumSeverity;

	extern CS_API bool printSeverity;
	extern CS_API bool printTimestamp;
	extern CS_API bool printTrace;

	extern CS_API bool isLoggingToFile;
	extern CS_API std::string logFileName;
	extern CS_API std::stringstream logFileBuffer;

	/// <summary>
	/// Allows for users to write custom logs
	/// </summary>
	/// <param name="owner">Log ownership (used for distinction)</param>
	/// <param name="severity">Severity of the log</param>
	/// <param name="message">This is run through std::format, taking ...args</param>
	/// <param name="...args">Format specifier arguments</param>
	template<typename... Args>
	void _UnclassifiedLog(std::string owner, std::string severity, std::string message, Args... args)
	{
		#ifndef CS_DISABLE_THREAD_SAFE_LOGGING
			std::scoped_lock lock(threadMutex);
		#endif
		std::stringstream ss;
		ss << owner.c_str();
		if (printTimestamp)
		{
			gt::Int64 time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			// %T is the format hh:mm:ss
			ss << "[" << std::put_time(std::localtime(&time), "%T") << "] ";
		}
		if (printSeverity) ss << severity.c_str();
		ss << std::vformat(message, std::make_format_args(std::forward<Args>(args)...)) << "\n";
		if (isLoggingToFile) logFileBuffer << ss.str();
		printf(ss.str().c_str());
	}

	/// <summary>
	/// Designed for logs that will either not appear in distribution builds or designed to output variables or current program states
	/// </summary>
	/// <param name="message">This is run through std::format, taking ...args</param>
	/// <param name="...args">Format specifier arguments</param>
	CS_DEFINE_LOGGER(Debug, Severity::Debug);
	/// <summary>
	/// Non-Essential logs that can often be disabled, alternative names: Verbose
	/// </summary>
	/// <param name="message">This is run through std::format, taking ...args</param>
	/// <param name="...args">Format specifier arguments</param>
	CS_DEFINE_LOGGER(Info, Severity::Info);
	/// <summary>
	/// Use this for the usual logging
	/// </summary>
	/// <param name="message">This is run through std::format, taking ...args</param>
	/// <param name="...args">Format specifier arguments</param>
	CS_DEFINE_LOGGER(Log, Severity::Log);
	/// <summary>
	/// Used to denote that something is done in a sub-optimal way or something can lead to an error down the road
	/// </summary>
	/// <param name="message">This is run through std::format, taking ...args</param>
	/// <param name="...args">Format specifier arguments</param>
	CS_DEFINE_LOGGER(Warn, Severity::Warn);
	/// <summary>
	/// Used to display recoverable errors where program state can resume as usual
	/// </summary>
	/// <param name="message">This is run through std::format, taking ...args</param>
	/// <param name="...args">Format specifier arguments</param>
	CS_DEFINE_LOGGER(Error, Severity::Error);
	/// <summary>
	/// Critical errors denote unrecoverable errors where the program must terminate
	/// </summary>
	/// <param name="message">This is run through std::format, taking ...args</param>
	/// <param name="...args">Format specifier arguments</param>
	CS_DEFINE_LOGGER(Critical, Severity::Critical);
	
	/// <summary>
	/// starts logging to a file; the file name will be <name><time>.log, Time being the date the file is saved
	/// </summary>
	/// <param name="name">Name of the log file</param>
	void CS_API BeginFileLog(std::string name);
	/// <summary>
	/// Writes pending the file log to file
	/// </summary>
	void CS_API EndFileLog();

	/// <summary>
	/// Only shows console calls for the given minimum or higher
	/// </summary>
	/// <param name="severity">Minimum severity to display</param>
	void CS_API SetMinimumSeverity(Severity severity);

	/// <summary>
	/// Toggles whether or not to shows the severities of console messages
	/// </summary>
	void CS_API ShowSeverities(bool enabled);
	/// <summary>
	/// Toggles whether or not to show timestamps for console messages
	/// </summary>
	void CS_API ShowTimestamps(bool enabled);
	/// <summary>
	/// Toggles whether or not to show traces (file name and line)
	/// </summary>
	void CS_API ShowTraces(bool enabled);

	/// <summary>
	/// Pauses the console and waits for user to type input
	/// </summary>
	/// <typeparam name="return_type">Customisable return type</typeparam>
	/// <param name="message">The message to ask the user</param>
	/// <returns>A const char* reference to response</returns>
	template <typename return_type = std::string*>
	void Ask(return_type output, std::string message)
	{
		std::cout << message;
		std::cin >> output;
	}
	/// <summary>
	/// Outputs a raw, unformatted string to the console, useful if you want to create your own messages or output raw data
	/// </summary>
	/// <param name="message">The message to output to the console, No formatting</param>
	template <typename any_type>
	void Output(any_type message)
	{
		std::scoped_lock lock(threadMutex);
		std::cout << message;
	}
}