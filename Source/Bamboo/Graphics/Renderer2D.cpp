
#include "Renderer2D.h"

#include "../Bamboo/Graphics/Shader.h"
#include "../Bamboo/Graphics/VertexArray.h"
#include "../Bamboo/Graphics/RendererCommand.h"
#include "../Bamboo/Math/Matrix3.h"
#include "../Bamboo/Math/Matrix4.h"

namespace Bamboo
{
    /**三角形顶点 */
    struct TriangleVertex
    {
        Vector3 position;
        Color color;
    };

    /**矩形顶点 */
    struct QuadVertex {
        Vector3 position;
        Color color;
    };

    struct Renderer2DData
    {
        ///最大三角形数量
        static const uint32_t MaxTriangles = 1;
        ///最大顶点数量
        static const uint32_t MaxVertices = MaxTriangles *3;
        ///最大索引数量
        static const uint32_t MaxIndices = MaxVertices * 3;

        Ref<VertexArray> TriangleVertexArray;
        Ref<VertexBuffer> TriangleBuffer;
        Ref<Shader> TriangleShader;
        Vector3 TriangleVertexPositions[3];
        TriangleVertex *TriangleVertices = nullptr;
        TriangleVertex *TriangleVerticesPtr = nullptr;
        uint32_t TriangleIndexCount = 0;


        Ref<VertexArray> QuadVertexArray;
        Ref<VertexBuffer> QuadBuffer;
        Ref<Shader> QuadShader;

        Vector4 QuadVertexPosition[4];

    };

    static Renderer2DData s_Data;

    void Renderer2D::Init()
    {
        //Triangle 
        s_Data.TriangleVertexArray = VertexArray::Create();
        s_Data.TriangleBuffer = VertexBuffer::Create(sizeof(TriangleVertex));

        s_Data.TriangleBuffer->SetLayout({
            {ShaderDatatType::Float3,"a_WorldPosition"},
            {ShaderDatatType::Float4,"a_Color"},
        });

        s_Data.TriangleVertices = new TriangleVertex[3];
        
        uint32_t trianglesIndices[3] = {0, 1, 2};
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(trianglesIndices, sizeof(trianglesIndices) / sizeof(trianglesIndices[0]));
        s_Data.TriangleVertexArray->SetIndexBuffer(indexBuffer);

        s_Data.TriangleShader = Shader::Create("triangle", "BambooAssets/Shaders/triangle.vert", "BambooAssets/Shaders/triangle.frag");

        s_Data.TriangleVertexArray->AddVertexBuffer(s_Data.TriangleBuffer);

        //增加顶点 数据
        s_Data.TriangleVertexPositions[0] = {-0.5, -0.5, 0.0};
        s_Data.TriangleVertexPositions[1] = {0.5, -0.5, 0.0};
        s_Data.TriangleVertexPositions[2] = {0.0, 0.5, 0.0};


        //Quad
        s_Data.QuadVertexArray = VertexArray::Create();
        s_Data.QuadBuffer = VertexBuffer::Create(sizeof(QuadVertex));
        s_Data.QuadBuffer->SetLayout({
            {ShaderDatatType::Float3,"a_WorldPosition"},
            {ShaderDatatType::Float4,"a_Color"}
            });

        s_Data.QuadVertexPosition[0] = { -0.5f,-0.5f,0.0f,1.0f };
        s_Data.QuadVertexPosition[1] = { 0.5f,-0.5f,0.0f,1.0f };
        s_Data.QuadVertexPosition[2] = { 0.5f,0.5f,0.0f,1.0f };
        s_Data.QuadVertexPosition[3] = { -0.5f,0.5f,0.0f,1.0f };
        //顶点数据，围成一个矩形
        uint32_t quadIndices[] = { 0,1,2,2,3,0 };
        Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(quadIndices[0]));
        s_Data.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);

        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadBuffer);

    }

    void Renderer2D::BeginScene()
    {
        StartBatch();
    }

    void Renderer2D::EndScene()
    {

        if(s_Data.TriangleIndexCount >0) {
            uint32_t dataSize = std::distance(s_Data.TriangleVertices, s_Data.TriangleVerticesPtr)*sizeof(*s_Data.TriangleVertices);

            s_Data.TriangleBuffer->SetData(s_Data.TriangleVertices, dataSize);
            
            s_Data.TriangleVertexArray->Bind();
            s_Data.TriangleShader->Bind();
            
            RendererCommand::DrawIndexed(s_Data.TriangleVertexArray, s_Data.TriangleIndexCount);
        }
    }

    void Renderer2D::StartBatch()
    {
        s_Data.TriangleIndexCount = 0;
        s_Data.TriangleVerticesPtr = s_Data.TriangleVertices;
    }

    void Renderer2D::DrawTriangle(const Vector3 &position, const Color &color)
    {

        Vector3 inPos = Vector3(position.x/1280,position.y/720,0);

         //Matrix4 m4 = Matrix4::Translate(inPos);
        float scale = 0.5f;
        Matrix3 m3 = Matrix3::Scale(scale, scale, scale);

        for(int i = 0;i<3;i++){
            s_Data.TriangleVerticesPtr->position = m3* s_Data.TriangleVertexPositions[i] ;
            s_Data.TriangleVerticesPtr->color = color;
            s_Data.TriangleVerticesPtr++;
        }
        s_Data.TriangleIndexCount += 3;

    }
}