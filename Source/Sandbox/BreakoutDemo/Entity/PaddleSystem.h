#pragma once
#include "../Bamboo/ECS/System/ISystem.h"

class PaddleSystem : public Bamboo::ISystem
{
    public:
        PaddleSystem();
        ~PaddleSystem();
        virtual void Update(entt::registry& registry, float deltaTime)  override;
};