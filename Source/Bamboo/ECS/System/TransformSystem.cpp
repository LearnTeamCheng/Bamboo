
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

            // if (!transform.Dirty)
            // {
            //     continue;
            // }
            // transform.LocalMatrix = Matrix4::Translate(transform.Position) * Matrix4::Scale(transform.Scale);
            // transform.Dirty = false;
        
            auto translation = Matrix4::Translate(transform.Position);
            // auto rotation =Matrix4::RotateXYZ(transform.Rotation);
            Matrix4 rotation ; //Matrix4::RotateXYZ(transform.Rotation);
            auto scale = Matrix4::Scale(transform.Scale);

            //平移、旋转、缩放矩阵相乘
            transform.LocalMatrix = translation  * scale;
            // 世界矩阵 = 本地矩阵的逆矩阵
            transform.WorldMatrix = transform.LocalMatrix ;
    
          
            transform.Dirty = false;
        }
    }
}