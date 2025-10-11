#pragma once
#include "../Bamboo/ECS/System.h"
class BallSystem :public ISystem
{
public:
    BallSystem(/* args */);
    ~BallSystem();
    virtual void Update(entt::registry& registry, float deltaTime) override;
};

