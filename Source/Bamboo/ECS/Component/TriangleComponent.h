#pragma once
#include "../Bamboo/Math/Color.h"

namespace Bamboo 
{
    struct TriangleComponent {
        
        Color TriangleColor = Color::White;

        TriangleComponent() = default;
        TriangleComponent(const TriangleComponent& other) = default;
    };
}