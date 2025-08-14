
#include "TransformSystem.h"
#include "../Bamboo/ECS/Component/TransformComponent.h"
namespace Bamboo {
    void TransformSystem::Update(entt::registry &registry, float deltaTime) {
        auto view = registry.view<TransformComponent>();
        for(auto entity : view){
            auto& transform = view.get<TransformComponent>(entity);
            transform.Position.x += 1.0f * deltaTime;
            transform.Position.y += 1.0f * deltaTime;
            transform.Position.z += 1.0f * deltaTime;
        }
    }
}