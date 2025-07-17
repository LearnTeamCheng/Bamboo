
#include "TransformSystem.h"
namespace Bamboo {
    void TransformSystem::Update(entity::registry &registry, float deltaTime) {
        auto view = registry.view<TransformComponent>();
        for(auto entity : view){
            auto& transform = view.get<TransformComponent>(entity);
            transform.Position.x += 1.0f * deltaTime;
            transform.Position.y += 1.0f * deltaTime;
            transform.Position.z += 1.0f * deltaTime;
        }
    }
}