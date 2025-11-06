#pragma once
#include "../Bamboo/Physics/PhysicsDefine.h"
namespace Bamboo
{

    struct RigidbodyComponent
    {
        /// @brief  刚体类型
        RigidbodyType Type;
        float Mass;
        /// @brief 力
        Vector3 Force;
        /// @brief  力矩
        Vector3 Torque;
        /// @brief  冲量
        Vector3 Impulse;
        /// @brief   摩擦力
        Vector2 Friction;
    };
}
