#include "Entity.hpp"
#include "Components/Behaviours.hpp"

CS_NAMESPACE_BEGIN
{
	void Entity::AddBehaviour(const std::string& name)
	{
		if (!HasComponent<Behaviours>())
			EmplaceComponent<Behaviours>();

		Behaviours& behaviours = GetComponent<Behaviours>();
		Behaviour* behaviour = ScriptStorage::CreateScript(name, *this);

		if (behaviour == nullptr)
		{
			cs_std::console::error("Script \"", name, "\: does not exist");
			return;
		}

		behaviours.AddBehaviour(behaviour);
	}
}