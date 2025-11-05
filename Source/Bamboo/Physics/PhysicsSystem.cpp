#include "PhysicsSystem.h"
#include "../Bamboo/ECS/Component/RigidbodyComponent.h"
#include "../Bamboo/ECS/Component/BoxCollider2DComponent.h"
namespace Bamboo::Physics
{
    void PhysicsSystem::Init()
    {
    
    }

    void PhysicsSystem::Update(entt::registry &registry, float deltaTime)
    {
        auto view = registry.view<RigidbodyComponent, BoxCollider2DComponent>();
        for (auto entity : view)
        {
            auto &[rigidbody, collider] = view.get<RigidbodyComponent, BoxCollider2DComponent>(entity);
        }
    }
}