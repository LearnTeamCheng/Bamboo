#pragma once
#include "../Bamboo/ECS/System/ISystem.h"

class PaddleSystem : public Bamboo::ISystem
{
    public:
        virtual void Update(Bamboo::SystemContext &context, float deltaTime)  override;
};