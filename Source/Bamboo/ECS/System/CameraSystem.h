#pragma once
#include "ISystem.h"

namespace Bamboo
{
    class CameraSystem : public ISystem
    {
        virtual void Update(entt::registry& registry, float deltaTime) override;
    };
};