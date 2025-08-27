
#include "Renderer2D.h"

#include "../Bamboo/Graphics/Shader.h"
#include "../Bamboo/Graphics/VertexArray.h"
#include "../Bamboo/Graphics/RendererCommand.h"

namespace Bamboo
{

    struct TriangleVertex
    {
        Vector3 position;
        Color color;
    };

    struct Renderer2DData
    {
        static const uint32_t MaxVertices = 2000;
        Ref<VertexArray> TriangleVertexArray;
        Ref<VertexBuffer> TriangleBuffer;
        Ref<Shader> TriangleShader;
        TriangleVertex *TriangleVertices;
        TriangleVertex *TriangleVerticesPtr;
        uint32_t TriangleIndexCount = 0;

      
    };

    static Renderer2DData s_Data;

    void Renderer2D::Init()
    {
        s_Data.TriangleVertexArray = VertexArray::Create();
        s_Data.TriangleBuffer = VertexBuffer::Create(sizeof(TriangleVertex) * 3);

        float vertices[3][3] = {
           {0.5f,0.1f,0},
           {-0.5f,0.1f,0},
           {0,0.5f,0},
        };
 

        s_Data.TriangleVertices = new TriangleVertex[3];

        s_Data.TriangleVertexArray->AddVertexBuffer(s_Data.TriangleBuffer);

        s_Data.TriangleBuffer->SetData(vertices, sizeof(vertices));

        uint32_t trianglesIndices[3] = {0, 1, 2};
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(trianglesIndices, sizeof(trianglesIndices) / sizeof(trianglesIndices[0]));
        s_Data.TriangleVertexArray->SetIndexBuffer(indexBuffer);

        // s_Data.TriangleShader = Shader::Create("assets/shaders/triangle.vert", "assets/shaders/triangle.frag");

        s_Data.TriangleVertexArray->AddVertexBuffer(s_Data.TriangleBuffer);
    }

    void Renderer2D::BeginScene()
    {
        StartBatch();
    }

    void Renderer2D::EndScene()
    {
        s_Data.TriangleVertexArray->Bind();
        s_Data.TriangleShader->Bind();
        
        RendererCommand::DrawIndexed(s_Data.TriangleVertexArray,s_Data.TriangleIndexCount);
    }

    void Renderer2D::StartBatch()
    {
        s_Data.TriangleIndexCount = 0;
        s_Data.TriangleVerticesPtr = s_Data.TriangleVertices;
    }

    void Renderer2D::DrawTriangle()
    {
    
    }
}