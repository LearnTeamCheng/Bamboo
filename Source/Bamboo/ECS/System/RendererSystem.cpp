#include "RendererSystem.h"
#include "../Bamboo/ECS/Component/TriangleComponent.h"
#include "../Bamboo/Graphics/Renderer2D.h"
#include "../Bamboo/Graphics/RendererCommand.h"



#include "../Bamboo/Core/Log.h"

namespace Bamboo
{
    void RendererSystem::Update(entt::registry &registry, float deltaTime)
    {
        //BAMBOO_CORE_INFO("Ren dererSystem");

        RendererCommand::SetClearColor({ 0.2f, 0.3f, 0.3f, 1.0f});
        RendererCommand::Clear();

        Renderer2D::BeginScene();
   
        auto view = registry.view< TriangleComponent>();
        for (auto entity : view)
        {
            auto triangle = view.get(entity);
            Renderer2D::DrawTriangle(Vector3::One, triangle.TriangleColor);

            //BAMBOO_CORE_INFO("TriangleComponent ");
        }
        Renderer2D::EndScene();
    }
}