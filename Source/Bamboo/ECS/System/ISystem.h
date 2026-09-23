#pragma once

#include "../SystemPhase.h"

namespace Bamboo
{
    struct SystemContext;
    class ISystem
    {
    public:
        virtual void Init() {}
        virtual void Update(SystemContext &context, float deltaTime) = 0;
        virtual ~ISystem() = default;
        virtual SystemPhase GetPhase() const { return SystemPhase::Logic; }
    };
}