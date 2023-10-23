#include "ECS.hpp"

namespace Crescendo::Engine
{
	entt::registry EntityManager::registry = {};
	entt::registry& Entity::registry = EntityManager::registry;
}