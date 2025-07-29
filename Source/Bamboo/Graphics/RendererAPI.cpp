#include "Bamboo/Graphics/RendererAPI.h"

#include "Bamboo/GraphicsAPI/OpenGL/OpenGLRendererAPI.h"

namespace Bamboo
{
    RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

    RendererAPI::Scope<RendererAPI>  RendererAPI::GetAPI(){
        switch (s_API)
        {
        case RendererAPI::API::None:
            
            return nullptr;
        
        case RendererAPI::API::OpenGL:
            return CreateScope<OpenGLRendererAPI>();
        default:
            break;
        }

        return nullptr;
    }
}