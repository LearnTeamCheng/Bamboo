
#include "../Bamboo/ECS/Component/Component.h"
#include "../Bamboo/Graphics/Camera.h"
#include "SpriteRendererSystem.h"
#include "../Bamboo/Math/Vector3.h"


namespace Bamboo
{

    void SpriteRendererSystem::Update(entt::registry &registry, float deltaTime)
    {
        Camera* mainCamera = nullptr;

        {
            auto view = registry.view<CameraComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto& [camera, transform] = view.get<CameraComponent, TransformComponent>(entity);
                mainCamera = &camera.CurrentCamera;
                break;
            }
        }

        if (mainCamera == nullptr) {
            return;
        }

        {
            auto view = registry.view<TransformComponent, SpriteRendererComponent>();
            
            float left = -mainCamera->GetOrthographicSize() * mainCamera->GetAspecatRatio() * 0.5f;
            float right = mainCamera->GetOrthographicSize() * mainCamera->GetAspecatRatio() * 0.5f;
            float top = mainCamera->GetOrthographicSize() * 0.5f;
            float bottom = -mainCamera->GetOrthographicSize() * 0.5f;

            float worldWidth = right - left;
            float worldHeight = top - bottom;
            float viewportWidht = mainCamera->GetViewportWidth();
            float viewPortHeight = mainCamera->GetViewportHeight();

            for (auto entity : view)
            {
                auto& [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

                float x = left + transform.Position.x / viewportWidht * worldWidth;
                float y = bottom + transform.Position.y / viewPortHeight * worldHeight;

                float sx = (sprite.Size.x / viewportWidht) * worldWidth;
                float sy = (sprite.Size.y / viewPortHeight) * worldHeight;

                transform.SetLocalToWorldMatrix(Vector3(x,y,0),Vector3(sx,sy,1.0));

            }
        }
    }
}