#include <vector>

#include "RendererSystem.h"

#include "../Bamboo/ECS/Component/Component.h"
#include "../Bamboo/Graphics/Renderer2D.h"
#include "../Bamboo/Graphics/RendererCommand.h"

#include "../Bamboo/ECS/Entity.h"

#include "../Bamboo/Graphics/Camera.h"

#include "../Bamboo/Core/Log.h"

namespace Bamboo
{
    void RendererSystem::Update(entt::registry &registry, float deltaTime)
    {
        // BAMBOO_CORE_INFO("Ren dererSystem");

        RendererCommand::SetClearColor({0.2f, 0.3f, 0.3f, 1.0f});
        RendererCommand::Clear();

        Camera *mainCamera = nullptr;
        {

            auto view = registry.view<CameraComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto &[camera, transform] = view.get<CameraComponent, TransformComponent>(entity);
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
                auto &[triangle, transform] = view.get<TriangleComponent, TransformComponent>(entity);
                Renderer2D::DrawTriangle(transform.Position, triangle.TriangleColor);
            }
        }

        // quad
        {
            auto view = registry.view<QuadComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto &[quad, transform] = view.get<QuadComponent, TransformComponent>(entity);

                Renderer2D::DrawQuad(transform.Position, Vector2(100, 100), quad.Color);
            }
        }

        // sprite
        {
            auto view = registry.view<SpriteRendererComponent, TransformComponent>();
            size_t size = view.size();
            std::vector<std::tuple<int, SpriteRendererComponent *, TransformComponent *>> sprites;
            for (auto entity : view)
            {
                auto &[sprite, transform] = view.get<SpriteRendererComponent, TransformComponent>(entity);
                // Renderer2D::DrawSprite(transform.LocalMatrix, sprite.SpriteColor, sprite.SpriteTexture);
                sprites.emplace_back(sprite.ZOrder, &sprite, &transform);
            }

            // 根据Zorder 排序
            std::sort(sprites.begin(), sprites.end(), [](const auto &a, const auto &b)
                      { return std::get<0>(a) < std::get<0>(b); });

            for (auto &[zOrder, sprite, transform] : sprites)
            {
                if (sprite->SpriteTexture == nullptr)
                {
                    sprite->SpriteTexture = Renderer2D::GetNormalTexture();
                }
                
                // auto viewMatrix = mainCamera->GetViewProjection() ;
                // auto model = viewMatrix* transform->WorldMatrix;
                //todo 这里可以优化 把精灵大小放到 transformSystem 里面 直接计算出世界矩阵 多一次循环，这里可以优化
                auto model = transform->WorldMatrix *Matrix4::Scale(sprite->Size);
                Renderer2D::DrawSprite(model, sprite->SpriteColor, sprite->SpriteTexture);
                // auto model = Matrix4::Translate(transform->Position) * Matrix4::Scale(sprite->Size);
                // Renderer2 D::DrawSprite(transform->WorldMatrix, sprite->SpriteColor, sprite->SpriteTexture);
            }
        }
        Renderer2D::EndScene();
    }
}