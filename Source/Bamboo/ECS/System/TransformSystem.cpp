
#include "TransformSystem.h"
#include "../Bamboo/ECS/Component/TransformComponent.h"
namespace Bamboo
{
    void TransformSystem::Update(entt::registry &registry, float deltaTime)
    {
        auto view = registry.view<TransformComponent>();
        for (auto entity : view)
        {
            auto &transform = view.get<TransformComponent>(entity);

            if (!transform.Dirty)
            {
                continue;
            }
            transform.LocalToWorldMatrix = Matrix4::Translate(transform.Position) * Matrix4::Scale(transform.Scale);
            transform.Dirty = false;
        }
    }
}