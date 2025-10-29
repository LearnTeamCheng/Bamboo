#pragma once
#include "entt.hpp"
namespace Bamboo {
    class ISystem {
        public:
            virtual void Init() {}
            virtual void Update(entt::registry& registry, float deltaTime) = 0;
            virtual~ISystem() = default;
    };
}