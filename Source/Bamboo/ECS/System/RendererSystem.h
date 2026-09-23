#pragma once
#include "../Bamboo/ECS/System/ISystem.h"

namespace Bamboo
{

    class RendererSystem : public ISystem
    {
    public:
        virtual void Update(SystemContext &context, float deltaTime) override;
        SystemPhase GetPhase() const override { return SystemPhase::Render; }
    };
}