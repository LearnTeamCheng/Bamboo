#pragma once
#include "../Bamboo/Core/Log.h"

#include "Scene.h"

#include "../Bamboo/ECS/Entity.h"
#include "../Bamboo/ECS/System/SpriteRendererSystem.h"
#include "../Bamboo/ECS/System/RendererSystem.h"
#include "../Bamboo/ECS/System/TransformSystem.h"
#include "../Bamboo/Physics/PhysicsSystem.h"
namespace Bamboo
{

    Scene::Scene()
    {
        BAMBOO_CORE_INFO("init scene");

        m_Systems.push_back(CreateScope<RendererSystem>());
        m_TransformSystem = CreateScope<TransformSystem>();
        m_SpriteRendererSystem = CreateScope<SpriteRendererSystem>();

        //m_PhysicsSystem = CreateScope<PhysicsSystem>();

        auto entity = CreateEntity("MainCamera");
        auto &cameraComponent = entity.AddComponent<CameraComponent>();
        cameraComponent.CurrentCamera = Camera();

        cameraComponent.CurrentCamera.SetOrthographic(10, 1, 100.0f);
        cameraComponent.CurrentCamera.SetViewportSize(1280, 720);
    }

    void Scene::Update(float deltaTime)
    {

        //更新物理 m_PhysicsSystem.Update(m_Registry, deltaTime);

        m_TransformSystem->Update(m_Registry, deltaTime);
        m_SpriteRendererSystem->Update(m_Registry, deltaTime);

        for (auto &system : m_Systems)
        {
            system->Update(m_Registry, deltaTime);
        }
    }

    Entity Scene::CreateEntity(const std::string &name)
    {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name)
    {
        Entity entity = {m_Registry.create(), this};
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<TagComponent>().Tag = name;
        m_EntityMap[uuid] = entity;
        return entity;
    }

    Entity Scene::FindEntityByName(const std::string_view &name)
    {
        auto view = m_Registry.view<TagComponent>();
        for (auto entity : view)
        {
            auto &tag = view.get<TagComponent>(entity);
            if (tag.Tag == name)
            {
                return {entity, this};
            }
        }

        return {};
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

    Camera *Scene::GetMainCamera()
    {
        auto entity = FindEntityByName("MainCamera");
        if (entity.HasComponent<CameraComponent>())
        {
            return &entity.GetComponent<CameraComponent>().CurrentCamera;
        }

        return nullptr;
    }

    Scene::~Scene()
    {
    }
};