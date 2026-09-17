#pragma once
#include "../Bamboo/Math/Color.h"
#include "../Bamboo/Math/Vector2.h"
namespace Bamboo::Shape {

    struct CircleComponent
    {
        Color CircleColor = Color::White;
        float Radius {10.0f};
        bool Fill {false;}

        CircleComponent() = default;
        CircleComponent(const CircleComponent& other) = default;
    };
    

};