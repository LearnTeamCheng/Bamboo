#pragma once
#include "../Bamboo/Math/Vector3.h"
namespace Bamboo::Physics
{
    class PhysicsWorld
    {
    public:
        PhysicsWorld() = default;
        ~PhysicsWorld() = default;

        // 重力设置
        void SetGravity(const Vector3 &gravity) { m_Gravity = gravity; }
        const Vector3 &GetGravity() const { return m_Gravity; }

        // void Step(float deltaTime);

    private:
        /// @brief 默认重力
        Vector3 m_Gravity = {0.0f, -9.8f, 0.0f};
    };
}