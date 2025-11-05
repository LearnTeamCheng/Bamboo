#pragma once
#include "../Bamboo/Math/Vector2.h"
namespace Bamboo
{
    struct BoxCollider2DComponent
    {
        Vector2 Size;
        Vector2 Offset;

        /// @brief 摩擦力
        float  Friction;
        /// @brief 密度
        float Density;
        /// @brief 是否为触发器
        bool IsTrigger;
    };

}