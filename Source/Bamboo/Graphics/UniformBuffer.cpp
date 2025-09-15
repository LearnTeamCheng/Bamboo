#include "UniformBuffer.h"
#include "../Bamboo/Core/Log.h"
#include "../Bamboo/Graphics/RendererAPI.h"
#include "../Bamboo/GraphicsAPI/OpenGL/OpenGLUniformBuffer.h"

namespace Bamboo
{

    Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
    {
        switch (RendererAPI::GetAPI())
        {

        case RendererAPI::API::None:
            BAMBOO_CORE_ERROR("RendererAPI::None is currently not supported!");
            return nullptr;
        case RendererAPI::API::OpenGL:
            return CreateRef<OpenGLUniformBuffer>(size, binding);
        }

        BAMBOO_CORE_ERROR("Unknown RendererAPI!");
        return nullptr;
    }
}