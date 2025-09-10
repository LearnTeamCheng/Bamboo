#include "RendererSystem.h"

#include "../Bamboo/ECS/Component/Component.h"
#include "../Bamboo/Graphics/Renderer2D.h"
#include "../Bamboo/Graphics/RendererCommand.h"

#include "../Bamboo/ECS/Entity.h"



#include "../Bamboo/Core/Log.h"

namespace Bamboo
{
    void RendererSystem::Update(entt::registry &registry, float deltaTime)
    {
        //BAMBOO_CORE_INFO("Ren dererSystem");

        RendererCommand::SetClearColor({ 0.2f, 0.3f, 0.3f, 1.0f});
        RendererCommand::Clear();

        Renderer2D::BeginScene();
        {
            auto view = registry.view< TriangleComponent, TransformComponent>();
            for (auto entity : view)
            {


                auto& [triangle, transform] = view.get< TriangleComponent, TransformComponent>(entity);


                Renderer2D::DrawTriangle(transform.Position, triangle.TriangleColor);

                //BAMBOO_CORE_INFO("TriangleComponent ");
            }

        }

        //quad
        {
            auto view = registry.view< QuadComponent, TransformComponent>();
            for (auto entity : view) 
            {
                auto& [quad, transform] = view.get<QuadComponent, TransformComponent>(entity);

                Renderer2D::DrawQuad(transform.Position,Vector2(100,100),quad.Color);
            }
        }

        Renderer2D::EndScene();
    }
}