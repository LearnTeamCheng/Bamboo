#include "Scene.h"
#include "../Core/Log.h"
#include "../ECS/Entity.h"
#include "../ECS/System/RendererSystem.h"
#include "../ECS/System/TransformSystem.h"
#include "../ECS/System/CameraSystem.h"
#include "../Physics/PhysicsSystem.h"
namespace Bamboo
{

    Scene::Scene()
    {
        BAMBOO_CORE_INFO("init scene");

        //这些先放在这里 后续编辑器反序列化时 自动加这些系统
        m_SystemRegistry.Register<TransformSystem>();
        m_SystemRegistry.Register<CameraSystem>();
        m_SystemRegistry.Register<Physics::PhysicsSystem>();
        m_SystemRegistry.Register<RendererSystem>();


        auto entity = CreateEntity("MainCamera");
        auto &cameraComponent = entity.AddComponent<CameraComponent>();
        cameraComponent.Primary = true;

        auto &transform = entity.GetComponent<TransformComponent>();
        transform.Position = Vector3(0.0f, 0.0f, 10.0f);


        cameraComponent.CurrentCamera.SetOrthographic(10, 1, 100.0f);
        cameraComponent.CurrentCamera.SetViewportSize(1280, 720);


    }

    void Scene::Update(float deltaTime)
    {
        // 顺序 logic Trasnform  Physics   Renderer
        m_SystemRegistry.UpdateLogic(m_Registry, deltaTime);   
        m_SystemRegistry.UpdateTransform(m_Registry, deltaTime);
        m_SystemRegistry.UpdatePhysics(m_Registry, deltaTime);
        m_SystemRegistry.UpdateRender(m_Registry, deltaTime);

    }

    Entity Scene::CreateEntity(const std::string &name)
    {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name)
    {
        Entity entity = {m_Registry.create(), this};
        entity.AddComponent<IDComponent>(uuid);
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
        m_EntityMap.erase(entity.GetUUID());
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