
#include "../Bamboo/ECS/Component/Component.h"
#include "../Bamboo/Graphics/Camera.h"
#include "SpriteRendererSystem.h"
#include "../Bamboo/Math/Vector3.h"
#include "../Bamboo/Core/Assert.h"

namespace Bamboo
{

    void SpriteRendererSystem::Update(entt::registry &registry, float deltaTime)
    {
        Camera *mainCamera = nullptr;

        {
            auto view = registry.view<CameraComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto &[camera, transform] = view.get<CameraComponent, TransformComponent>(entity);
                mainCamera = &camera.CurrentCamera;
                break;
            }
        }

        if (mainCamera == nullptr)
        {
            return;
        }

        {
            auto view = registry.view<TransformComponent, SpriteRendererComponent>();

            for (auto entity : view)
            {
                auto &[transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);
                Vector3 worldPos = mainCamera->ScreenToWorldPosition(transform.Position);
                Vector3 size = mainCamera->PixelSizeToWorldSize(sprite.Size);
                transform.SetLocalToWorldMatrix(worldPos,size);
            }
        }
    }
}