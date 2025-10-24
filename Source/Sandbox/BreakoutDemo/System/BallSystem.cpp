#include "BallSystem.h"
#include "../Bamboo/ECS/Component/Component.h"

#include "../Component/BallComponent.h"

void BallSystem::Update(entt::registry &registry, float deltaTime)
{
    auto view = registry.view<Bamboo::TransformComponent, BallComponent>();

    for (auto entity : view) {
    }
}