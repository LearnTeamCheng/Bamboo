#include "VertexArray.h"

#include "../GraphicsAPI/OpenGL/OpenGLVertexArray.h"



namespace Bamboo
{

    Ref<VertexArray> VertexArray::Create()
    {
        return CreateRef<OpenGLVertexArray>();
    }
}