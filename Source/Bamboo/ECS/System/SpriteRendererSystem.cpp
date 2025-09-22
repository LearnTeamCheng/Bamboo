#include "SpriteRendererSystem.h"

namespace Bamboo
{

    void SpriteRendererSystem::Update(entt::registry &registry, float deltaTime)
    {
        Camera* mainCamera = nullptr;

        {
            auto view = registry.view<CameraComponent, TransformComponent>();
            for (auto entity : view)
            {
            }
        }

        {
            auto view = registry.view<TransformComponent, SpriteRendererComponent>();
            for (auto entity : view)
            {
            
            }
        }
    }
}