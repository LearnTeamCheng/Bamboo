#include "VertexArray.h"

#include "../GraphicsAPI/OpenGL/OpenGLVertextArray.h"



namespace Bamboo
{

    Ref<VertexArray> VertexArray::Create()
    {
        return CreateRef<OpenGLVertexArray>();
    }
}