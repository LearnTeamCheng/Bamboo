#pragma once
#include "../Core/Ref.h"
#include "../ECS/Entity.h"
#include "../Math/Vector3.h"
#include "../ECS/System/ISystem.h"
#include "../Physics/PhysicsWorld.h"

namespace Bamboo::Physics
{
    class PhysicsSystem : public ISystem
    {
    public:
        void Init();
        virtual void Update(entt::registry &registry, float deltaTime);

        /// @brief 给实体施加力
        void ApplyForce(Entity entity, const Vector3 &force);
        
        /// @brief 给实体添加角力
        void ApplyTorque(Entity entity, const Vector3 &torque);
        
        /// @brief 给实体施加冲量
        void ApplyImpulse(Entity entity, const Vector3 &impulse);
        
        /// @brief 给实体施加阻力
        void ApplyTorqueImpulse(Entity entity, const Vector3 &torqueImpulse);
    private:
        Scope<PhysicsWorld> m_PhysicsWorld;
    };
}