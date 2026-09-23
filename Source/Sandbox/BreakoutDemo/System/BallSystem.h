#pragma once


#include "../Bamboo/ECS/System/ISystem.h"
class BallSystem :public Bamboo::ISystem
{
public:
    virtual void Update(Bamboo::SystemContext &context, float deltaTime) override;
};

