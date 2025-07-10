#include "ISystem.h"

namespace Bamboo {
    class TransformSystem : public ISystem {
        public:
            virtual void Update(float deltaTime);
    };
}