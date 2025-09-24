#include "../Bamboo/Graphics/RendererAPI.h"
#include "../Bamboo/Graphics/RendererCommand.h"
#include "Renderer.h"
#include "./Renderer2D.h"


namespace Bamboo
{

    void Renderer::Init()
    {
        RendererCommand::Init();
    }

    void Renderer::Shutdown()
    {
        Renderer2D::Shutdown();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height){
        //RendererAPI::SetViewport(0,0,width, height);
    }

} // namespace Bamboo
