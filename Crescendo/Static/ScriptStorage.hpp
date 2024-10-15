#pragma once
#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	struct Behaviour;
	class Entity;

	class ScriptStorage
	{
	public:
		static std::unordered_map<std::string, std::function<Behaviour* (Entity)>> scripts;
	public:
		static void RegisterScript(const std::string& name, std::function<Behaviour* (Entity)> script);
		static Behaviour* CreateScript(const std::string& name, Entity e);
	};
}