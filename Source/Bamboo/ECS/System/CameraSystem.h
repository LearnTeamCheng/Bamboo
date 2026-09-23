#pragma once
#include "ISystem.h"

namespace Bamboo
{
    class CameraSystem : public ISystem
    {
        virtual void Update(SystemContext &context, float deltaTime) override;
    };
};