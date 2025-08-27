#pragma once
#include "../Bamboo/ECS/System/ISystem.h"

namespace Bamboo
{

    class RendererSystem : public ISystem
    {
    public:
        virtual void Update(entt::registry &registry, float deltaTime) override;
    };
}