#include "VertexArray.h"

#include "../Bamboo/GraphicsAPI/OpenGL/OpenGLVertextArray.h"



namespace Bamboo
{

    Ref<VertexArray> VertexArray::Create()
    {
        return CreateRef<OpenGLVertextArray>();
    }
}