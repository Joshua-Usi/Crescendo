#pragma once
#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	struct Behaviour;

	class ScriptStorage
	{
	public:
		static std::unordered_map<std::string, std::function<Behaviour* ()>> scripts;
	public:
		static void RegisterScript(const std::string& name, std::function<Behaviour* ()> script)
		{
			scripts[name] = script;
		}
		static Behaviour* CreateScript(const std::string& name)
		{
			return (scripts.count(name) == 0) ? nullptr : scripts[name]();
		}
	};
}