#pragma once

#include "Core/common.hpp"

#include <unordered_map>

namespace Crescendo::Engine
{
	/// <summary>
	/// Stores global variables in CVar like system, all data is stored as strings
	/// Numbers are capable of 64-bit integers, 64-bit floats
	/// </summary>
	class CVar
	{
	private:
		// Name, Value
		static std::unordered_map<std::string, std::string> data;
	public:
		/// <summary>
		/// Register a CVar with the system
		/// </summary>
		/// <param name="name">Name of the CVar</param>
		/// <param name="value">Initial value, leave blank for empty</param>
		static void Register(const std::string& name, const std::string& value = "");
		/// <summary>
		/// Get the value of a CVar
		/// </summary>
		/// <typeparam name="T">template type to get CVar as</typeparam>
		/// <param name="name">Name of CVar</param>
		template <typename T = std::string>
		static T Get(const std::string& name);
		/// <summary>
		/// Get all the names of the registered CVars
		/// Useful for a auto-complete system
		/// </summary>
		/// <returns>A vector of strings</returns>
		static std::vector<std::string> GetNames();
		static void Set(const std::string& name, std::string value)
		{
			CS_ASSERT(data.count(name) != 0, "CVar does not exist / Was not registered");
			data[name] = value;
		}
		static void Set(const std::string& name, int64_t value)
		{
			CS_ASSERT(data.count(name) != 0, "CVar does not exist / Was not registered");
			data[name] = std::to_string(value);
		}
		static void Set(const std::string& name, double value)
		{
			CS_ASSERT(data.count(name) != 0, "CVar does not exist / Was not registered");
			data[name] = std::to_string(value);
		}
		/// <summary>
		/// Load a Cvar xml configuration file
		/// </summary>
		/// <param name="path">Path to config file</param>
		static void LoadConfigXML(const std::string& path);
		/// <summary>
		/// Serialize the configuration as an XML compatible string
		/// </summary>
		/// <returns>XML string of config</returns>
		static std::string SerializeConfigXML();
	};
}