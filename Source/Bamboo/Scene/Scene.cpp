#pragma once
#include "Scene.h"
namespace Bamboo
{

    Scene::Scene()
    {
    }


    void Scene::Update(float deltaTime) 
    {
        for (auto& system : m_Systems)
        {
           system->Update(m_Registry,deltaTime);
        }
        
    }

    Entity Scene::CreateEntity(const std::string &name)
    {
        return Entity(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name)
    {
        Entity entity = {m_Registry.create(),this};

        
        m_Entities[uuid] = entity;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity.GetHandle());
        m_Entities.erase(entity.GetUUID());
    }
};