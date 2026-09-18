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

        // TODO(物理): 物理步进（Step）尚未实现 —— 本类目前只是重力的容器，
        // 且 PhysicsSystem::Init 从未被调用；见 refactor_plan.md §4.2。

    private:
        /// @brief 默认重力
        Vector3 m_Gravity = {0.0f, -9.8f, 0.0f};
    };
}