#include "ECS.hpp"

CS_NAMESPACE_BEGIN
{
	entt::registry EntityManager::registry = {};
	entt::registry& Entity::registry = EntityManager::registry;
}