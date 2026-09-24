#pragma once
#include "../Bamboo/Math/Color.h"
#include "../Bamboo/Math/Vector2.h"
namespace Bamboo 
{
    struct TriangleComponent {
        
        Color color = Color::White;
        Vector2 size;

        TriangleComponent() = default;
        TriangleComponent(const TriangleComponent& other) = default;
    };
}