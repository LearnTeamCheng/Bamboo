#pragma once

#include "../Bamboo/Graphics/Camera.h"
namespace Bamboo 
{
    struct CameraComponent
    {
       Camera CurrentCamera;

       CameraComponent() = default;
       CameraComponent(const CameraComponent& ) = default;
    };
   
}