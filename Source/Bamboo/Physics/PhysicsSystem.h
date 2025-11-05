#pragma once
#include "../Bamboo/ECS/System/ISystem.h"

namespace Bamboo::Physics
{
    class PhysicsSystem : public ISystem
    {
    public:
        void Init();
        virtual void Update(entt::registry &registry, float deltaTime);
    };
}