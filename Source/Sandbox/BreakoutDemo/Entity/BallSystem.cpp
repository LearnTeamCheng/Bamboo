#include "BallSystem.h"
#include "BallComponent.h"
#include "../Bamboo/ECS/Component/Component.h"
void BallSystem::Update(entt::registry &registry, float deltaTime)
{
    auto view = registry.view<Bamboo::TransformComponent, BallComponent>();
}