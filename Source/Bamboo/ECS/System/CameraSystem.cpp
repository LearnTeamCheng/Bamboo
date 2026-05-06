#include "CameraSystem.h"

#include "../Component/CameraComponent.h"
#include "../Component/TransformComponent.h"

#include "../Entity.h"

namespace Bamboo
{

    void CameraSystem::Update(entt::registry &registry, float deltaTime)
    {
        Camera *mainCamera = nullptr;
        auto view = registry.view<CameraComponent, TransformComponent>();
        for (auto entity : view)
        {
            auto &camera = view.get<CameraComponent>(entity);
            auto &transform = view.get<TransformComponent>(entity);
            if (camera.Primary)
            {
                mainCamera = &camera.CurrentCamera;
                mainCamera->SetPosition(transform.Position);
            }
        }
    }
}