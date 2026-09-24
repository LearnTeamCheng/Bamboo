#pragma once
#include "../Bamboo/Math/Color.h"
#include "../Bamboo/Math/Vector2.h"
namespace Bamboo::Shape {

    struct CircleComponent
    {
        Color color = Color::White;
        float radius {10.0f};
        bool fill {false};

        CircleComponent() = default;
        CircleComponent(const CircleComponent& other) = default;
    };
    

};