#include <vector>

#include "RendererSystem.h"

#include "../Bamboo/ECS/Component/Component.h"
#include "../Bamboo/Graphics/Renderer2D.h"
#include "../Bamboo/Graphics/RendererCommand.h"

#include "../Bamboo/ECS/Entity.h"

#include "../Bamboo/Graphics/Camera.h"

#include "../Bamboo/Core/Log.h"
#include "../SystemContext.h"
#include "../World.h"

namespace Bamboo
{
    void RendererSystem::Update(SystemContext &context, float deltaTime)
    {
        // TODO(渲染): 清屏色硬编码在这里，应该来自场景/相机的渲染设置，见 P2-4。
        RendererCommand::SetClearColor({0.2f, 0.3f, 0.3f, 1.0f});
        RendererCommand::Clear();

        auto &registry = context.world.GetRegistry(); // 获取 ECS 注册表

        Camera *mainCamera = nullptr;
        {

            auto view = registry.view<CameraComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto [camera, transform] = view.get<CameraComponent, TransformComponent>(entity);
                mainCamera = &camera.CurrentCamera;
            }
        }

        if (mainCamera)
        {
            Renderer2D::BeginScene(*mainCamera);
        }
        else
        {
            Renderer2D::BeginScene();
        }

        // triangel
        {
            auto view = registry.view<TriangleComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto [triangle, transform] = view.get<TriangleComponent, TransformComponent>(entity);
                Renderer2D::DrawTriangle(transform.Position, triangle.TriangleColor);
            }
        }

        // quad
        {
            auto view = registry.view<QuadComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto [quad, transform] = view.get<QuadComponent, TransformComponent>(entity);

                Renderer2D::DrawQuad(transform.Position, Vector2(100, 100), quad.Color);
            }
        }

        // sprite
        {
            auto view = registry.view<SpriteRendererComponent, TransformComponent>();
            size_t size = view.size();
            // TODO(性能): 每帧构造 vector 并排序，属于堆分配 + O(n log n)。
            // 应改为复用成员缓冲，见 P2-4。
            std::vector<std::tuple<int, SpriteRendererComponent *, TransformComponent *>> sprites;
            for (auto entity : view)
            {
                auto [sprite, transform] = view.get<SpriteRendererComponent, TransformComponent>(entity);
                sprites.emplace_back(sprite.ZOrder, &sprite, &transform);
            }

            // 按 ZOrder 升序排序，保证绘制顺序
            std::sort(sprites.begin(), sprites.end(), [](const auto &a, const auto &b)
                      { return std::get<0>(a) < std::get<0>(b); });

            for (auto &[zOrder, sprite, transform] : sprites)
            {
                // ⚠️ 已知缺陷：这里在**渲染期间回写** ECS 数据，破坏了"渲染只读"的契约，
                // 而且会永久抹掉"用户没有设置纹理"这一信息。
                // 正确做法是在 DrawSprite 内部回落到白纹理，见 P0-8。
                if (sprite->SpriteTexture == nullptr)
                {
                    sprite->SpriteTexture = Renderer2D::GetNormalTexture();
                }

                // TODO(性能): 精灵的缩放每帧在这里现算；应在 TransformSystem 里
                // 直接产出最终世界矩阵，见 P2-4。
                auto model = transform->WorldMatrix * Matrix4::Scale(sprite->Size);
                Renderer2D::DrawSprite(model, sprite->SpriteColor, sprite->SpriteTexture);
            }
        }
        Renderer2D::EndScene();
    }
}