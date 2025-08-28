
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
        ///最大三角形数量
        static const uint32_t MaxTriangles = 1;
        ///最大顶点树
        static const uint32_t MaxVertices = MaxTriangles *3;

        static const uint32_t MaxIndices = MaxVertices * 3;

        Ref<VertexArray> TriangleVertexArray;
        Ref<VertexBuffer> TriangleBuffer;
        Ref<Shader> TriangleShader;
        Vector3 TriangleVertexPositions[3];
        TriangleVertex *TriangleVertices = nullptr;
        TriangleVertex *TriangleVerticesPtr = nullptr;
        uint32_t TriangleIndexCount = 0;

        float testVerices[9] = {};
    };

    static Renderer2DData s_Data;

    void Renderer2D::Init()
    {
        s_Data.TriangleVertexArray = VertexArray::Create();
        s_Data.TriangleBuffer = VertexBuffer::Create(sizeof(TriangleVertex));

        s_Data.TriangleBuffer->SetLayout({
            { ShaderDatatType::Float3,"a_WorldPosition"}
        });

        //s_Data.TriangleVertices = new TriangleVertex[3];
        


        s_Data.TriangleVertexArray->AddVertexBuffer(s_Data.TriangleBuffer);

        uint32_t trianglesIndices[3] = {0, 1, 2};
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(trianglesIndices, sizeof(trianglesIndices) / sizeof(trianglesIndices[0]));
        s_Data.TriangleVertexArray->SetIndexBuffer(indexBuffer);

        s_Data.TriangleShader = Shader::Create("triangle", "BambooAssets/Shaders/triangle.vert", "BambooAssets/Shaders/triangle.frag");

        s_Data.TriangleVertexArray->AddVertexBuffer(s_Data.TriangleBuffer);

        //增加顶点 数据
        s_Data.TriangleVertexPositions[0] = {-0.5, -0.5, 0.0};
        s_Data.TriangleVertexPositions[1] = {0.5, -0.5, 0.0};
        s_Data.TriangleVertexPositions[2] = {0.0, 0.5, 0.0};

        float testVerices[] = {

        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top  
        };

        for (int i = 0; i < 9; i++) {
            s_Data.testVerices[i] = testVerices[i];
        }
      
    }

    void Renderer2D::BeginScene()
    {
        StartBatch();
    }

    void Renderer2D::EndScene()
    {

        s_Data.TriangleBuffer->SetData( s_Data.testVerices,sizeof(s_Data.testVerices));
        s_Data.TriangleVertexArray->Bind();
        s_Data.TriangleShader->Bind();

        RendererCommand::DrawIndexed(s_Data.TriangleVertexArray, s_Data.TriangleIndexCount);
    }

    void Renderer2D::StartBatch()
    {
        s_Data.TriangleIndexCount = 0;
        s_Data.TriangleVerticesPtr = s_Data.TriangleVertices;
    }

    void Renderer2D::DrawTriangle(const Vector3 &position, const Color &color)
    {
        // s_Data.TriangleVerticesPtr->position = position;
        // s_Data.TriangleVerticesPtr->color = color;
        s_Data.TriangleBuffer->SetData( s_Data.testVerices,sizeof(s_Data.testVerices));
    }
}