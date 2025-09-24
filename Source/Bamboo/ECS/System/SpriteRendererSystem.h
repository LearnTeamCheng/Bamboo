#pragma once
#include "ISystem.h"

namespace Bamboo {
    class SpriteRendererSystem : public ISystem {
        public:
            virtual void Update(entt::registry& registry, float deltaTime) ;
    };
}