#pragma once
//#include "../Bamboo/ECS/System.h"
#include "../Bamboo/ECS/System/ISystem.h"
class BallSystem :public Bamboo::ISystem
{
public:
    BallSystem(/* args */);
    ~BallSystem();
    virtual void Update(entt::registry& registry, float deltaTime) override;
};

