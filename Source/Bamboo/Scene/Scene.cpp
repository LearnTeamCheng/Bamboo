#pragma once
#include"../Bamboo/Core/Log.h"

#include "Scene.h"



#include "../Bamboo/ECS/Entity.h"
#include "../Bamboo/ECS/System/SpriteRendererSystem.h"
#include"../Bamboo/ECS/System/RendererSystem.h"
#include "../Bamboo/ECS/System/TransformSystem.h"

namespace Bamboo
{

    Scene::Scene()
    {
        BAMBOO_CORE_INFO("init scene");

        //m_Systems.push_back(CreateScope<SpriteRendererSystem>());
        m_Systems.push_back(CreateScope<RendererSystem>());

        m_TransformSystem = CreateScope<TransformSystem>();
        m_SpriteRendererSystem = CreateScope<SpriteRendererSystem>();


        //auto entity = CreateEntity();
        //entity.AddComponent<TriangleComponet>();
 
    }

    void Scene::Update(float deltaTime) 
    {

        m_TransformSystem->Update(m_Registry,deltaTime);
        m_SpriteRendererSystem->Update(m_Registry,deltaTime);

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