#include "ScriptStorage.hpp"

CS_NAMESPACE_BEGIN
{
	std::unordered_map<std::string, std::function<Behaviour * ()>> ScriptStorage::scripts = {};
}