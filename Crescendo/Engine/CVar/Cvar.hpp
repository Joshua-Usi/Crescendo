#pragma once

#include "common.hpp"

#include <unordered_map>
#include <algorithm>

CS_NAMESPACE_BEGIN
{
	template<typename T>
	concept CVarSetType = std::integral<T> || std::floating_point<T> || std::is_same<T, std::string>::value || std::is_same<T, bool>::value;

	class CVar
	{
	private:
		static std::unordered_map<std::string, std::string> data;
	public:
		static void Register(const std::string& name, const std::string& value = "");
		static std::vector<std::string> GetNames();
	public:
		template<CVarSetType T>
		static T Get(const std::string& name)
		{
			CS_ASSERT(data.count(name) != 0, "CVar does not exist / Was not registered");
			// Bool first since it counts as an integral
			if constexpr (std::is_same<T, bool>::value)
			{
				// Capitalisation doesn't matter
				std::string value = data[name];
				std::transform(value.begin(), value.end(), value.begin(), ::tolower);
				return value == "true";
			}
			else if constexpr (std::integral<T>) return static_cast<T>(std::atoll(data[name].c_str()));
			else if constexpr (std::floating_point<T>) return static_cast<T>(std::atof(data[name].c_str()));
			else if constexpr (std::is_same<T, std::string>::value) return data[name];
			return T(); // Should never happen
		}
		template<CVarSetType T>
		static void Set(const std::string& name, T value)
		{
			CS_ASSERT(data.count(name) != 0, "CVar does not exist / Was not registered");
			// Bool first since it counts as an integral
			if constexpr (std::is_same<T, bool>::value) data[name] = value ? "true" : "false";
			else if constexpr (std::integral<T>) data[name] = std::to_string(value);
			else if constexpr (std::floating_point<T>) data[name] = std::to_string(value);
			else if constexpr (std::is_same<T, std::string>::value) data[name] = value;
		}
	public:
		static void LoadConfigXML(const std::string& path, bool clearRegistry = true);
		static std::string SerializeConfigXML();
	};
}