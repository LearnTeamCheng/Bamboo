
#include "TransformSystem.h"
#include "../Component/TransformComponent.h"
#include "../SystemContext.h"
#include "../World.h"
namespace Bamboo
{
    void TransformSystem::Update(SystemContext &context, float deltaTime)
    {
        auto view = context.world.GetRegistry().view<TransformComponent>();
        for (auto entity : view)
        {
            auto &transform = view.get<TransformComponent>(entity);

            // 注意：旋转当前被跳过（Matrix4::RotateXYZ 用错了角度制，见 P1-2），
            // 所以这里只合成 平移 × 缩放。
            auto translation = Matrix4::Translate(transform.Position);
            auto scale = Matrix4::Scale(transform.Scale);

            transform.LocalMatrix = translation * scale;

            // 当前没有父/子层级，因此"世界矩阵"就等于本地矩阵。
            // 注意：它**不是**本地矩阵的逆矩阵（早期注释写错了）。
            transform.WorldMatrix = transform.LocalMatrix;

            // TODO(ECS): Dirty 标志目前无条件清零，等于每帧全量重算；
            // 应该只在 Position/Rotation/Scale 被修改时置脏，见 P1-12。
            transform.Dirty = false;
        }
    }
}
