#pragma once

#include "../Bamboo/Graphics/Camera.h"
namespace Bamboo
{
    struct CameraComponent
    {
        Camera currentCamera;
        /// @brief 主相机
        bool primary = false;



        CameraComponent() = default;
        CameraComponent(const CameraComponent &) = default;
    };

}