#include "Entity.h"

namespace Bamboo 
{
	Entity::Entity(entt::entity handle, World* world) :m_EntityHandle(handle), m_World(world) 
	{
		
	}

	bool Entity::IsValid() const{
		return m_World != nullptr && m_World->GetRegistry().valid(m_EntityHandle);
	}
}    