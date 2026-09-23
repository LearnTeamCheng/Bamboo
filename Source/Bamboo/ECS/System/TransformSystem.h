#pragma once
#include "ISystem.h"

namespace Bamboo
{
    class TransformSystem : public ISystem
    {
    public:
        void Update(SystemContext &context, float deltaTime);
        SystemPhase GetPhase() const override { return SystemPhase::Transform; }
    };
}