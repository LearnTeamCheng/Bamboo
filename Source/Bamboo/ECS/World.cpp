#include "World.h"
#include "Entity.h"
namespace Bamboo
{
    Entity World::CreateEntity()
    {
        Entity entity = {m_Registry.create(), this};
        return entity;
    }

    void World::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

};