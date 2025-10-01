

#include "Renderer2D.h"

#include "../Bamboo/Core/Assert.h"
#include "../Bamboo/Graphics/Shader.h"
#include "../Bamboo/Graphics/VertexArray.h"
#include "../Bamboo/Graphics/RendererCommand.h"
#include "../Bamboo/Math/Matrix3.h"
#include "../Bamboo/Math/Matrix4.h"

#include "../Bamboo/Graphics/Camera.h"
#include "../Bamboo/Graphics/UniformBuffer.h"


#include <array>

namespace Bamboo
{
    /**三角形顶点 */
    struct TriangleVertex
    {
        Vector3 position;
        Color color;
    };

    /**矩形顶点 */
    struct QuadVertex
    {
        Vector3 position;
        Color color;
    };

    /**精灵顶点数据结构 */
    struct SpriteVertex
    {
        Vector3 Position;
        Color Color;
        // 纹理坐标
        Vector2 TexCoord;
        float TexIndex;
    };

    struct Renderer2DData
    {
        /// 最大三角形数量
        static const uint32_t MaxTriangles = 1;
        /// 最大顶点数量
        static const uint32_t MaxVertices = MaxTriangles * 3;
        /// 最大索引数量
        static const uint32_t MaxIndices = MaxVertices * 3;
        /// 最大纹理槽数量
        static const uint32_t MaxTextureSlots = 32;

        static const uint32_t QuadVertexCount = 4;
        static const uint32_t MaxQuadIndices = 6;

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
        QuadVertex *QuadVertices = nullptr;
        QuadVertex *QuadVerticesPtr = nullptr;

        uint32_t QuadIndexCount = 0;

        // Sprite
        Ref<VertexArray> SpriteVertexArray;
        Ref<VertexBuffer> SpriteBuffer;
        Ref<Shader> SpriteShader;
        Vector3 SpriteVertexPositions[4];
        SpriteVertex *SpriteVertices = nullptr;
        SpriteVertex *SpriteVerticesPtr = nullptr;
        uint32_t SpriteIndexCount = 0;

        uint32_t SpriteCount = 0;

        uint32_t TextureSlotIndex = 1;

        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;

        struct CameraData
        {
            Matrix4 ViewProjectionMatrix;
        };

        Ref<UniformBuffer> CameraUniformBuffer;
    };

    static Renderer2DData s_Data;

    void Renderer2D::Init()
    {
        // Triangle
        s_Data.TriangleVertexArray = VertexArray::Create();
        s_Data.TriangleBuffer = VertexBuffer::Create(sizeof(TriangleVertex));

        s_Data.TriangleBuffer->SetLayout({
            {ShaderDatatType::Float3, "a_WorldPosition"},
            {ShaderDatatType::Float4, "a_Color"},
        });

        s_Data.TriangleVertices = new TriangleVertex[3];

        uint32_t trianglesIndices[3] = {0, 1, 2};
        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(trianglesIndices, sizeof(trianglesIndices) / sizeof(trianglesIndices[0]));
        s_Data.TriangleVertexArray->SetIndexBuffer(indexBuffer);

        s_Data.TriangleShader = Shader::Create("triangle", "BambooAssets/Shaders/triangle.vert", "BambooAssets/Shaders/triangle.frag");

        s_Data.TriangleVertexArray->AddVertexBuffer(s_Data.TriangleBuffer);

        // 增加顶点 数据
        s_Data.TriangleVertexPositions[0] = {-0.5, -0.5, 0.0};
        s_Data.TriangleVertexPositions[1] = {0.5, -0.5, 0.0};
        s_Data.TriangleVertexPositions[2] = {0.0, 0.5, 0.0};

        // Quad
        s_Data.QuadVertexArray = VertexArray::Create();
        s_Data.QuadBuffer = VertexBuffer::Create(sizeof(QuadVertex));
        s_Data.QuadBuffer->SetLayout({{ShaderDatatType::Float3, "a_WorldPosition"},
                                      {ShaderDatatType::Float4, "a_Color"}});

        s_Data.QuadVertices = new QuadVertex[4];
        s_Data.QuadVertexPosition[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPosition[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPosition[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPosition[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
        // 顶点数据，围成一个矩形
        // uint32_t quadIndices[] = { 0,1,2,2,3,0 };

        uint32_t *quadIndices = new uint32_t[6 * 2];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < 2 * s_Data.QuadIndexCount; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, 2);
        s_Data.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);

        delete[] quadIndices;

        s_Data.QuadShader = Shader::Create("Quad", "BambooAssets/Shaders/triangle.vert", "BambooAssets/Shaders/triangle.frag");
        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadBuffer);

        // Sprite
        s_Data.SpriteVertexArray = VertexArray::Create();
        s_Data.SpriteBuffer = VertexBuffer::Create(sizeof(SpriteVertex) * 2);
        s_Data.SpriteBuffer->SetLayout({{ShaderDatatType::Float3, "a_Position"},
                                        {ShaderDatatType::Float4, "a_Color"},
                                        {ShaderDatatType::Float2, "a_TexCoord"},
                                        {ShaderDatatType::Float, "a_TexIndex"}

        });

        s_Data.SpriteVertices = new SpriteVertex[s_Data.QuadVertexCount * 2];
        s_Data.SpriteVertexPositions[0] = {-0.5f, -0.5f, 0.0f};
        s_Data.SpriteVertexPositions[1] = {0.5f, -0.5f, 0.0f};
        s_Data.SpriteVertexPositions[2] = {0.5f, 0.5f, 0.0f};
        s_Data.SpriteVertexPositions[3] = {-0.5f, 0.5f, 0.0f};

        // 顶点数据，围成一个矩形
        //  uint32_t spriteIndices[] = { 0,1,2,2,3,0 };
        uint32_t *spriteIndices = new uint32_t[s_Data.MaxQuadIndices * 2];
        offset = 0;

        for (int i = 0; i < 2 * s_Data.MaxQuadIndices; i += 6)
        {
            spriteIndices[i + 0] = offset + 0;
            spriteIndices[i + 1] = offset + 1;
            spriteIndices[i + 2] = offset + 2;

            spriteIndices[i + 3] = offset + 2;
            spriteIndices[i + 4] = offset + 3;
            spriteIndices[i + 5] = offset + 0;
            offset += 4;
        }

        Ref<IndexBuffer> spriteIndexBuffer = IndexBuffer::Create(spriteIndices, 2 * 6);
        s_Data.SpriteVertexArray->SetIndexBuffer(spriteIndexBuffer);
        delete[] spriteIndices;

        s_Data.SpriteShader = Shader::Create("Sprite", "BambooAssets/Shaders/sprite.vert", "BambooAssets/Shaders/sprite.frag");
        s_Data.SpriteVertexArray->AddVertexBuffer(s_Data.SpriteBuffer);

        s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
        //白色纹理
        s_Data.TextureSlots[0] =  Texture2D::Create(TextureSpecification());
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.TextureSlots[0]->SetData(&whiteTextureData,sizeof(uint32_t));
    }

    void Renderer2D::BeginScene()
    {
        StartBatch();
    }

    void Renderer2D::BeginScene(const Camera &camera)
    {
        Matrix4 viewProjection = camera.GetProjection();
        s_Data.CameraUniformBuffer->SetData(&viewProjection, sizeof(Renderer2DData::CameraData));
        StartBatch();
    }

    void Renderer2D::EndScene()
    {
        Flush();
    }

    void Renderer2D::StartBatch()
    {
        s_Data.TriangleIndexCount = 0;
        s_Data.TriangleVerticesPtr = s_Data.TriangleVertices;

        s_Data.QuadIndexCount = 0;
        s_Data.QuadVerticesPtr = s_Data.QuadVertices;

        s_Data.SpriteCount = 0;
        s_Data.SpriteIndexCount = 0;
        s_Data.SpriteVerticesPtr = s_Data.SpriteVertices;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer2D::Flush()
    {
        if (s_Data.TriangleIndexCount > 0)
        {
            uint32_t dataSize = std::distance(s_Data.TriangleVertices, s_Data.TriangleVerticesPtr) * sizeof(*s_Data.TriangleVertices);

            s_Data.TriangleBuffer->SetData(s_Data.TriangleVertices, dataSize);

            s_Data.TriangleVertexArray->Bind();
            s_Data.TriangleShader->Bind();

            RendererCommand::DrawIndexed(s_Data.TriangleVertexArray, s_Data.TriangleIndexCount);
        }

        if (s_Data.QuadIndexCount > 0)
        {
            uint32_t dataSize = std::distance(s_Data.QuadVertices, s_Data.QuadVerticesPtr) * sizeof(*s_Data.QuadVertices);
            s_Data.QuadBuffer->SetData(s_Data.QuadVertices, dataSize);

            s_Data.QuadVertexArray->Bind();
            s_Data.QuadShader->Bind();

            RendererCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
        }

        if (s_Data.SpriteIndexCount > 0)
        {
            uint32_t dataSize = std::distance(s_Data.SpriteVertices, s_Data.SpriteVerticesPtr) * sizeof(*s_Data.SpriteVertices);
            s_Data.SpriteBuffer->SetData(s_Data.SpriteVertices, dataSize);
            // 绑定纹理
            for (int i = 0; i <s_Data.TextureSlotIndex; i++)
            {
                s_Data.TextureSlots[i]->Bind(i);
            }

            s_Data.SpriteVertexArray->Bind();
            s_Data.SpriteShader->Bind();
            RendererCommand::DrawIndexed(s_Data.SpriteVertexArray, s_Data.SpriteIndexCount);
        }
    }

    void Renderer2D::DrawTriangle(const Vector3 &position, const Color &color)
    {

        Vector3 inPos = Vector3(position.x / 1280, position.y / 720, 0);

        // Matrix4 m4 = Matrix4::Translate(inPos);
        float scale = 0.5f;
        Matrix3 m3 = Matrix3::Scale(scale, scale, scale);

        for (int i = 0; i < 3; i++)
        {
            s_Data.TriangleVerticesPtr->position = m3 * s_Data.TriangleVertexPositions[i];
            s_Data.TriangleVerticesPtr->color = color;
            s_Data.TriangleVerticesPtr++;
        }
        s_Data.TriangleIndexCount += 3;
    }

    void Renderer2D::DrawQuad(const Vector2 &position, const Vector2 &size, const Color &color)
    {
        for (int i = 0; i < 4; i++)
        {
            s_Data.QuadVerticesPtr->position = s_Data.QuadVertexPosition[i];
            s_Data.QuadVerticesPtr->color = color;
            s_Data.QuadVerticesPtr++;
        }
        s_Data.QuadIndexCount += 6;
    }

    void Renderer2D::DrawQuad(const Vector3 &position, const Vector2 &size, const Color &color)
    {
        for (int i = 0; i < 4; i++)
        {
            s_Data.QuadVerticesPtr->position = s_Data.QuadVertexPosition[i];
            s_Data.QuadVerticesPtr->color = color;
            s_Data.QuadVerticesPtr++;
        }
        s_Data.QuadIndexCount += 6;
    }

    void Renderer2D::DrawSprite(const Matrix4 &localMatrix, const Color &color, Ref<Texture2D> &texture)
    {
        // 纹理坐标
        Vector2 TexCoord[] = {
            {0.0f, 0.0f}, // 左下
            {1.0f, 0.0f}, // 右下
            {1.0f, 1.0f}, // 右上
            {0.0f, 1.0f}  // 左上
        };

        constexpr size_t spriteVertexCount = 4;

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i] == *texture){
                textureIndex=(float)i;
                break;
            }
        }

        if(textureIndex == 0.0f){
            if(s_Data.TextureSlotIndex >= s_Data.MaxTextureSlots){
                NextBatch();
            }

            textureIndex = (float)s_Data.TextureSlotIndex;

            BAMBOO_ASSESERT(s_Data.TextureSlotIndex >=s_Data.TextureSlots.size(),"Texture slot index out of range");
            s_Data.TextureSlots.at(s_Data.TextureSlotIndex) = texture;
            s_Data.TextureSlotIndex++;
        }
        


        for (int i = 0; i < spriteVertexCount; i++)
        {
            s_Data.SpriteVerticesPtr->Position = localMatrix * Vector4(s_Data.SpriteVertexPositions[i]);

            s_Data.SpriteVerticesPtr->Color = color;
            s_Data.SpriteVerticesPtr->TexCoord = TexCoord[i];
            s_Data.SpriteVerticesPtr->TexIndex = (float)(s_Data.SpriteCount  +1);
            s_Data.SpriteVerticesPtr++;
        }

        // if (s_Data.SpriteCount < s_Data.MaxTextureSlots)
        // {
        //     s_Data.TextureSlots[s_Data.SpriteCount] = texture;
        // }

        s_Data.SpriteIndexCount += 6;
        s_Data.SpriteCount++;
    }

    void Renderer2D::Shutdown()
    {
        delete[] s_Data.TriangleVertices;
        delete[] s_Data.QuadVertices;
        delete[] s_Data.SpriteVertices;
    }
}