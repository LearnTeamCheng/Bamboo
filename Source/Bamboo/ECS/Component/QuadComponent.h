#pragma once
#include "../Bamboo/Math/Vector3.h"
#include "../Bamboo/Math/Color.h"

#include "../Bamboo/Math/Vector2.h"

namespace Bamboo {

    //@brief 四边形组件
    struct QuadComponent
    {
        Color color;
        Vector2 size;
        QuadComponent() =default;
        QuadComponent(const QuadComponent&) = default;
    };
}