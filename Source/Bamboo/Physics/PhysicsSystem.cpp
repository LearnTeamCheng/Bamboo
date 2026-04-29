#include "PhysicsSystem.h"

#include "../Bamboo/ECS/Component/RigidbodyComponent.h"
#include "../Bamboo/ECS/Component/BoxCollider2DComponent.h"
#include "../ECS/Component/TransformComponent.h"
namespace Bamboo::Physics
{
    void PhysicsSystem::Init()
    {
        m_PhysicsWorld = CreateScope<PhysicsWorld>();

        m_PhysicsWorld->SetGravity(Vector3(0.0f, -9.8f, 0.0f));
    }

    void PhysicsSystem::Update(entt::registry &registry, float deltaTime)
    {
        auto view = registry.view<RigidbodyComponent,TransformComponent>();
        for (auto entity : view)
        {
            auto &[rigidbody,transform] = view.get<RigidbodyComponent,TransformComponent>(entity);
            // 动态物体 才受力的影响
            if (rigidbody.Type == RigidbodyType::Dynamic)
            {
                

            }
            else if (rigidbody.Type == RigidbodyType::Static)
            {   
                //todo 静态物体 只参加碰撞检测
            }
        }
    }

    void PhysicsSystem::ApplyForce(Entity entity, const Vector3 &force)
    {
        entity.GetComponent<RigidbodyComponent>().Force += force;
    }

    void PhysicsSystem::ApplyTorque(Entity entity, const Vector3 &torque)
    {
        entity.GetComponent<RigidbodyComponent>().Torque += torque;
    }

    void PhysicsSystem::ApplyImpulse(Entity entity, const Vector3 &impulse)
    {
        entity.GetComponent<RigidbodyComponent>().Impulse += impulse;
    }

    void PhysicsSystem::ApplyTorqueImpulse(Entity entity, const Vector3 &torqueImpulse)
    {
    }
}