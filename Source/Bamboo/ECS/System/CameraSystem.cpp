#include "CameraSystem.h"

#include "../Component/CameraComponent.h"
#include "../Component/TransformComponent.h"

#include "../Entity.h"

#include "../SystemContext.h"
#include "../World.h"

namespace Bamboo
{

    void CameraSystem::Update(SystemContext &context, float deltaTime)
    {
        auto view = context.world.GetRegistry().view<CameraComponent, TransformComponent>();
        for (auto entity : view)
        {
            auto &camera = view.get<CameraComponent>(entity);
            auto &transform = view.get<TransformComponent>(entity);
            if (camera.Primary)
            {
                // 视图矩阵 = 相机变换的逆。这里手写"平移到负位置 + 绕 Z 反向旋转"，
                // 只在"纯平移 + 单轴 Z 旋转"时等价于真正的求逆。
                // TODO(相机): 多轴旋转或缩放相机时这里会算错；
                // 应改用 transform.WorldMatrix.Inverse()，见 P1-13。
                Matrix4 translation = Matrix4::Translate(-transform.Position);
                Matrix4 rotation = Matrix4::RotateZ(-transform.Rotation.z);
                camera.CurrentCamera.SetView(translation * rotation);
            }
        }
    }
}
