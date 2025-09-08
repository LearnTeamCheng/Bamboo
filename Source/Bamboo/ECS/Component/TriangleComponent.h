#pragma once
#include "../Bamboo/Math/Color.h"
#include "../Bamboo/Math/Vector2.h"
namespace Bamboo 
{
    struct TriangleComponent {
        
        Color TriangleColor = Color::White;
        Vector2 Size;

        TriangleComponent() = default;
        TriangleComponent(const TriangleComponent& other) = default;
    };
}