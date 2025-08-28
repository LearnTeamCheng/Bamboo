#include "./Graphics/RendererAPI.h"

#include "./GraphicsAPI/OpenGL/OpenGLRendererAPI.h"

namespace Bamboo
{
    RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

    Scope<RendererAPI>  RendererAPI::Create(){
        // switch (s_API)
        // {
        // case RendererAPI::API::None:
            
        //     return nullptr;
        
        // case RendererAPI::API::OpenGL:
            return CreateScope<OpenGLRendererAPI>();
        // default:
        //     break;
        // }

        // return nullptr;
    }
}