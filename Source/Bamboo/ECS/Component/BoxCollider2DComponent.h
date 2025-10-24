#pragma once
#include "../Bamboo/Math/Vector2.h"
namespace Bamboo
{
    struct BoxCollider2DComponent
    {
        Vecto2 Size;
        Vecto2 Offset;

        /// @brief 摩擦力
        float  Friction;

        /// @brief 密度
        float Density;
    };

}