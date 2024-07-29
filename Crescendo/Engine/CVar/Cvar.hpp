#pragma once

#include "common.hpp"

#include <unordered_map>
#include <algorithm>
#include <variant>

CS_NAMESPACE_BEGIN
{
	using CVarType = std::variant<int64_t, double, std::string, bool>;

	template<typename T>
	concept CVarSetType = std::integral<T> || std::floating_point<T> || std::is_same<T, std::string>::value || std::is_same<T, bool>::value;

	class CVar
	{
	private:
		static std::unordered_map<std::string, CVarType> data;
	public:
		template<CVarSetType T>
		static void Register(const std::string& name, const std::string& value = "")
		{
			if constexpr (std::is_same<T, bool>::value)
			{
				// Capitalisation doesn't matter
				std::string temp = value;
				std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
				data.emplace(name, temp == "true");
			}
			else if constexpr (std::integral<T>) data.emplace(name, std::atoll(value.c_str()));
			else if constexpr (std::floating_point<T>) data.emplace(name, std::atof(value.c_str()));
			else if constexpr (std::is_same<T, std::string>::value) data.emplace(name, value);
		}
		static std::vector<std::string> GetNames();
	public:
		template<CVarSetType T>
		static T Get(const std::string& name)
		{
			CS_ASSERT(data.count(name) != 0, "CVar with the name \"" + name + "\" does not exist / Was not registered");

			const CVarType& value = data[name];

			if constexpr (std::is_same<T, bool>::value) return std::get<bool>(value);
			else if constexpr (std::integral<T>) return static_cast<T>(std::get<int64_t>(value));
			else if constexpr (std::floating_point<T>) return static_cast<T>(std::get<double>(value));
			else if constexpr (std::is_same<T, std::string>::value) return std::get<std::string>(value);
		}
		template<CVarSetType T>
		static void Set(const std::string& name, T value)
		{
			CS_ASSERT(data.count(name) != 0, "CVar does not exist / Was not registered");
			// Bool first since it counts as an integral
			if constexpr (std::is_same<T, bool>::value) data[name] = value;
			else if constexpr (std::integral<T>) data[name] = static_cast<int64_t>(value);
			else if constexpr (std::floating_point<T>) data[name] = static_cast<double>(value);
			else if constexpr (std::is_same<T, std::string>::value) data[name] = value;
		}
	public:
		static void LoadConfigXML(const std::string& path, bool clearRegistry = true);
		static std::string SerializeConfigXML();
	};
}