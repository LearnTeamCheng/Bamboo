#pragma once
#include "ISystem.h"

namespace Bamboo
{
    class TransformSystem : public ISystem
    {
    public:
        void Update(entt::registry &registry, float deltaTime);
    };
}