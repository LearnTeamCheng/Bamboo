#pragma once
#include "../Bamboo/Math/Vector2.h"
namespace Bamboo
{   
    /// @brief 2D盒子碰撞器组件
    struct BoxCollider2DComponent
    {
        Vector2 Size;
        Vector2 Offset;
        /// @brief 摩擦力
        float Friction;
        /// @brief 密度
        float Density;
        /// @brief 是否为触发器
        bool IsTrigger;

        BoxCollider2DComponent() = default;
        BoxCollider2DComponent(const BoxCollider2DComponent &) = default;
        BoxCollider2DComponent(Vector2 size, Vector2 offset = Vector2::Zero, float friction = 0.0f, float density = 1.0f, bool isTrigger = false) : Size(size), Offset(offset), Friction(friction), Density(density), IsTrigger(isTrigger)
        {
        }
    };

}