#include "ScriptStorage.hpp"
#include "ECS/Entity.hpp"

CS_NAMESPACE_BEGIN
{
	std::unordered_map<std::string, std::function<Behaviour * (Entity e)>> ScriptStorage::scripts = {};
	void ScriptStorage::RegisterScript(const std::string& name, std::function<Behaviour* (Entity e)> script)
	{
		scripts[name] = script;
	}
	Behaviour* ScriptStorage::CreateScript(const std::string& name, Entity e)
	{
		return (scripts.count(name) == 0) ? nullptr : scripts[name](e);
	}
}