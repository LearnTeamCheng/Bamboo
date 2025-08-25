#pragma once
#include"../Bamboo/Core/Log.h"

#include "../Bamboo/ECS/Entity.h"
#include "Scene.h"
#include "../Bamboo/ECS/System/SpriteRendererSystem.h"

namespace Bamboo
{

    Scene::Scene()
    {
        BAMBOO_CORE_INFO("init scene");

        m_Systems.push_back(std::make_shared<SpriteRendererSystem>());
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
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name)
    {
        Entity entity = { m_Registry.create(),this};
        
        m_EntityMap[uuid] = entity;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
        //m_EntityMap.erase(entity)
    }

    Scene::~Scene() {

    }
};