#include "BallSystem.h"

void BallSystem::Update(entt::registry &registry, float deltaTime)
{
    auto view = register.view<TransformComponent, BallComponent>();
}