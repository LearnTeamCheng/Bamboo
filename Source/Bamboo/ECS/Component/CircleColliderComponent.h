#pragma once
#include "../Bamboo/Math/Vector2.h"
namespace Bamboo {
    struct CircleColliderComponent
    {
        float Radius;
        Vector2 Center;
        Vector2 Offset;
        bool IsTrigger;
    };
    
}