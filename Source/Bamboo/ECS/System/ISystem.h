#pragma once
#include "entt.hpp"
namespace Bamboo
{

    enum class SystemPhase
    {
        Logic,
        Render,
        Physics,
        Transform,
    };

    class ISystem
    {
    public:
        virtual void Init() {}
        virtual void Update(entt::registry &registry, float deltaTime) = 0;
        virtual ~ISystem() = default;
        virtual  SystemPhase GetPhase() const { return SystemPhase::Logic; } 
    };
}