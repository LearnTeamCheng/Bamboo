#include "ISystem.h"

namespace Bamboo
{
    class TransformSystem : public ISystem
    {
    public:
        void Update(entity::registry &registry, float deltaTime);
    };
}