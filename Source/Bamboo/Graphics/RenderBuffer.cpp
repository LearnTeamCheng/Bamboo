#include "RenderBuffer.h"

#include "../Bamboo/Graphics/OpenGLBuffer.h"
namespace Bamboo
{
    Ref<IndexBuffer> IndexBuffer::Create(uint32_t size)
    {
        return CreateRef<OpenGLIndexBuffer>(size);
    }

    Ref<VertexBuffer> VertexBuffer::Create(uint32_t count)
    {
        return CreateRef<OpenGLVertexBuffer>(count);
    }
    
    Ref<VertexBuffer> VertexBuffer::Create(uint32_t *vertices, uint32_t count)
    {
        return CreateRef<OpenGLVertexBuffer>(vertices, count);
    }
};