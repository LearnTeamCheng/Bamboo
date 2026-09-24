#include "Scene.h"
#include "../Core/Log.h"
#include "../ECS/Entity.h"
#include "../ECS/System/RendererSystem.h"
#include "../ECS/System/TransformSystem.h"
#include "../ECS/System/CameraSystem.h"
#include "../Physics/PhysicsSystem.h"
#include "../ECS/SystemContext.h"
namespace Bamboo
{

    Scene::Scene()
    {
        BAMBOO_CORE_INFO("init scene");

        // 这些先放在这里 后续编辑器反序列化时 自动加这些系统
        m_SystemRegistry.Register<TransformSystem>();
        m_SystemRegistry.Register<CameraSystem>();
        m_SystemRegistry.Register<Physics::PhysicsSystem>();
        m_SystemRegistry.Register<RendererSystem>();

        auto entity = CreateEntity("MainCamera");
        auto &cameraComponent = entity.AddComponent<CameraComponent>();
        cameraComponent.primary = true;

        auto &transform = entity.GetComponent<TransformComponent>();
        transform.position = Vector3(0.0f, 0.0f, 10.0f);

        cameraComponent.currentCamera.SetOrthographic(10, 1, 100.0f);
        cameraComponent.currentCamera.SetViewportSize(1280, 720);
    }

    void Scene::Update(float deltaTime)
    {
        // 顺序 logic Trasnform  Physics   Renderer
        SystemContext context{ m_World};
        m_SystemRegistry.Update(context, deltaTime);
    }

    Entity Scene::CreateEntity(const std::string &name)
    {
        return CreateEntityWithUUID(UUID::Generate(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name)
    {
        Entity entity = m_World.CreateEntity();
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<TagComponent>().tag = name;
        m_EntityMap[uuid] = entity;
        return entity;
    }

    Entity Scene::FindEntityByName(const std::string_view &name)
    {
        auto view = m_World.GetRegistry().view<TagComponent>();
        for (auto entity : view)
        {
            auto &tag = view.get<TagComponent>(entity);
            if (tag.tag == name)
            {
                // return {entity, this};
                return {entity, &m_World};
            }
        }

        return {};
    }

    void Scene::DestroyEntity(Entity entity)
    {
        UUID uuid = entity.GetUUID();
        m_World.DestroyEntity(entity);
        m_EntityMap.erase(uuid);
    }

    Camera *Scene::GetMainCamera()
    {
        auto entity = FindEntityByName("MainCamera");
        if (entity.HasComponent<CameraComponent>())
        {
            return &entity.GetComponent<CameraComponent>().currentCamera;
        }

        return nullptr;
    }

    Scene::~Scene()
    {
    }
};