
#include "BRenderer2D.h"

#include "../Bamboo/Graphics/Shader.h"
#include "../Bamboo/Graphics/VertexArray.h"

namespace Bamboo
{

    struct TriangleVertex
    {
        Vector3 position;
        Color color;
    }

    struct Renderer2DData
    {
        Ref<VertexArray> TriangleVertexArray;
        Ref<VertexBuffer> TriangleBuffer;
        Ref<Shader> TriangleShader;
    };

    void Renderer2d::Init()
    {
    
    }
}