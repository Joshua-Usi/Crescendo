#pragma once

#include <chrono>
#include <iostream>
#include <mutex>
#include <format>

namespace Crescendo::Engine
{
	class Console
	{
	public:
		enum class Severity : uint8_t { Debug = 0, Info = 1, Log = 2, Warn = 3, Error = 4, Critical = 5};
	private:
		inline static constexpr const char* const SeverityString[6]
		{
			"Debug",
			"Info",
			"Log",
			"Warn",
			"Error",
			"Critical",
		};
	private:
		inline static bool enableThreadSafety = true;
		inline static std::mutex threadMutex;

		inline static Severity minimumLoggedSeverity = Severity::Info;
		
		inline static bool printSeverity = true;
		inline static bool printTimestamp = true;
		// TODO add traces when non-terminal variadic arguments come out
		inline static bool printTrace = false;

		inline static bool isLoggingToFile = false;
		inline static std::string logFileName = "";
		inline static std::stringstream logFileBuffer;
		/// <summary>
		/// Allows for users to write custom logs
		/// </summary>
		/// <param name="owner">Log ownership (used for distinction)</param>
		/// <param name="severity">Severity of the log</param>
		/// <param name="message">This is run through std::format, taking ...args</param>
		/// <param name="...args">Format specifier arguments</param>
		template<typename... Args>
		inline static void _UnclassifiedLog(const std::string& owner, Severity severity, const std::string& message, Args... args)
		{
			if (minimumLoggedSeverity > severity) return;
			if (enableThreadSafety) std::scoped_lock lock(threadMutex);
			std::stringstream ss;
			ss << owner << " ";
			if (printTimestamp)
			{
				int64_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				// %T is the format hh:mm:ss
				ss << "[" << std::put_time(std::localtime(&time), "%T") << "] ";
			}
			if (printSeverity) ss << "[" << SeverityString[static_cast<size_t>(severity)] << "] ";
			ss << std::vformat(message, std::make_format_args(std::forward<Args>(args)...)) << "\n";
			if (isLoggingToFile) logFileBuffer << ss.str();
			std::cout << ss.str();
		}
	public:
		/// <summary>
		/// Highly verbose logs, Usually not displayed
		/// </summary>
		template<typename... Args>
		inline static void Debug(std::string message, Args... args) { _UnclassifiedLog("(Crescendo)", Severity::Debug, message, args...); }
		/// <summary>
		/// Informational logs
		/// </summary>
		template<typename... Args>
		inline static void Info(std::string message, Args... args) { _UnclassifiedLog("(Crescendo)", Severity::Info, message, args...); }
		/// <summary>
		/// Standard logs
		/// </summary>
		template<typename... Args>
		inline static void Log(std::string message, Args... args) { _UnclassifiedLog("(Crescendo)", Severity::Log, message, args...); }
		/// <summary>
		/// Usually signifies potentially uintended behaviour or side effects
		/// </summary>
		template<typename... Args>
		inline static void Warn(std::string message, Args... args) { _UnclassifiedLog("(Crescendo)", Severity::Warn, message, args...); }
		/// <summary>
		/// A recoverable error occured
		/// </summary>
		template<typename... Args>
		inline static void Error(std::string message, Args... args) { _UnclassifiedLog("(Crescendo)", Severity::Error, message, args...); }
		/// <summary>
		/// An unrecoverable error occured and the program must terminate
		/// </summary>
		template<typename... Args>
		inline static void Critical(std::string message, Args... args) { _UnclassifiedLog("(Crescendo)", Severity::Critical, message, args...); }
	
		/// <summary>
		/// starts logging to a file; the file name will be <name><time>.log, Time being the date the file is saved
		/// </summary>
		/// <param name="name">Name of the log file</param>
		static void BeginFileLog(const std::string& name);
		/// <summary>
		/// Writes pending the file log to file
		/// </summary>
		static void EndFileLog();

		/// <summary>
		/// Only shows console calls for the given minimum or higher
		/// </summary>
		/// <param name="severity">Minimum severity to display</param>
		static void SetMinimumSeverity(Severity severity);

		/// <summary>
		/// Enables / Disables thread safety for console messages
		/// If output order doesn't matter, this can be disabled for a performance boost
		/// </summary>
		/// <param name="enabled"></param>
		static void SetThreadSafety(bool enabled);

		/// <summary>
		/// Toggles whether or not to shows the severities of console messages
		/// </summary>
		static void ShowSeverities(bool enabled);
		/// <summary>
		/// Toggles whether or not to show timestamps for console messages
		/// </summary>
		static void ShowTimestamps(bool enabled);
		/// <summary>
		/// Toggles whether or not to show traces (file name and line)
		/// </summary>
		static void ShowTraces(bool enabled);

		/// <summary>
		/// Pauses the console and waits for user to type input
		/// </summary>
		/// <typeparam name="return_type">Customisable return type</typeparam>
		/// <param name="message">The message to ask the user</param>
		/// <returns>A const char* reference to response</returns>
		template <typename return_type>
		inline static return_type Ask(const char* message)
		{
			return_type output;
			std::cout << message;
			std::cin >> output;
			return output;
		}
		/// <summary>
		/// Outputs a raw, unformatted string to the console, useful if you want to create your own messages or output raw data
		/// </summary>
		/// <param name="message">The message to output to the console, No formatting</param>
		template <typename any_type>
		inline static void Output(any_type message)
		{
			if (enableThreadSafety) std::scoped_lock lock(threadMutex);
			std::cout << message;
		}
	};
}