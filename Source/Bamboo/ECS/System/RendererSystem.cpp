#include "RendererSystem.h"
#include "../Bamboo/ECS/Component/TriangleComponnet.h"
#include "../Bamboo/Graphics/Renderer2D.h"


#include "../Bamboo/Core/Log.h"

namespace Bamboo
{
    void RendererSystem::Update(entt::registry &registry, float deltaTime)
    {
        //BAMBOO_CORE_INFO("RendererSystem");
   
        auto view = registry.view< TriangleComponent>();
        for (auto entity : view)
        {
            //auto triangle = view.get(entity);
            //Renderer2D::DrawTriangle(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2]);

            //BAMBOO_CORE_INFO("TriangleComponent ");
        }
        //Renderer2D::EndScene();
    }
}