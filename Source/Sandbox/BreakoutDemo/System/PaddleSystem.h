#pragma once
#include "../Bamboo/ECS/System/ISystem.h"

class PaddleSystem : public Bamboo::ISystem
{
    public:
        virtual void Update(entt::registry& registry, float deltaTime)  override;
};