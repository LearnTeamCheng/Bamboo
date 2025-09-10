#include "../Bamboo/Graphics/RendererAPI.h"
#include "../Bamboo/Graphics/RendererCommand.h"
#include "Renderer.h"


namespace Bamboo
{

    void Renderer::Init()
    {
        RendererCommand::Init();
    }

    void Renderer::Shutdown()
    {
    
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height){
        //RendererAPI::SetViewport(0,0,width, height);
    }

} // namespace Bamboo
