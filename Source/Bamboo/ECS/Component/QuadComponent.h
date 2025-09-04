#pragma once
#include "../Bamboo/Math/Vector3.h"
#include "../Bamboo/Math/Color.h"

#include "../Bamboo/Math/Vector2.h"

namespace Bamboo {

    struct QuadComponent
    {
        Vector3 Position;
        Vector3 Color;
        Vector2 Size;

        QuadComponent() =default;
        QuadComponent(const QuadComponent&) = default;
    };
}