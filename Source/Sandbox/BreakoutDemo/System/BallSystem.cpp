#include "BallSystem.h"
#include "../Bamboo/ECS/Component/Component.h"

#include "../Component/BallComponent.h"
#include "../Bamboo/ECS/SystemContext.h"
#include "../Bamboo/ECS/World.h"

void BallSystem::Update(Bamboo::SystemContext &context, float deltaTime)
{   
    
    auto view = context.world.GetRegistry().view<Bamboo::TransformComponent, BallComponent>();

    for (auto entity : view) {
    }
}