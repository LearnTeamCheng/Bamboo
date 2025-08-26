
#include "Renderer2D.h"

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

        static const uint32_t MaxVertices = 2000;
    };


    static Renderer2DData s_Data;

    void Renderer2d::Init()
    {
        s_Data.TriangleVertexArray = VertexArray::Create();
        s_Data.TriangleBuffer = VertexBuffer::Create(sizeof(TriangleVertex) * 3);

        float vertices[3][3] = {
            { { -0.5f, -0.5f, 0.0f }, { 255, 0, 0, 255 } },
            { {  0.5f, -0.5f, 0.0f }, { 0, 255, 0, 255 } },
            { {  0.0f,  0.5f, 0.0f }, { 0, 0, 255, 255 } }
        }

        s_Data.TriangleVertexArray->AddVertexBuffer(s_Data.TriangleBuffer);

        s_Data.TriangleBuffer->SetData(vertices, sizeof(vertices));
        
        int trianglesIndices[3] = { 0, 1, 2 };
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(trianglesIndices,sizeof(trianglesIndices)/sizeof(trianglesIndices[0]));
        s_Data.TriangleVertexArray->SetIndexBuffer(indexBuffer);

        // s_Data.TriangleShader = Shader::Create("assets/shaders/triangle.vert", "assets/shaders/triangle.frag");
        
        s_Data.TriangleVertexArray->AddBuffer(s_Data.TriangleBuffer, indexBuffer);

    }
}