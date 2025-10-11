#pragma once
#include "entt.hpp"
namespace Bamboo {
    class ISystem {
        public:
            // virtual void OnCreate(entt::registry& registry) = 0;
            // virtual void OnDestroy(entt::registry& registry) = 0;
            virtual void Update(entt::registry& registry, float deltaTime) = 0;
            virtual~ISystem() = default;
    };
}