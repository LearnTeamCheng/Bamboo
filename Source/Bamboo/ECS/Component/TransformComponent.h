#pragma once

#include "../../Math/Vector3.h"
#include "../../Math/Matrix4.h"
namespace Bamboo{
    struct TransformComponent
    {
        Vector3 Position = {0.0f,0.0f,0.0f};
        Vector3 Rotation = {0.0f,0.0f,0.0f};
        Vector3 Scale = {1.0f,1.0f,1.0f};

        TransformComponent() = default;
        TransformComponent(const TransformComponent& other) = default;
        
        bool 
    };
    
}