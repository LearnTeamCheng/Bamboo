#pragma once
#include "PrimitiveBatch.h"
#include <unordered_map>
#include <array>

#include "../Bamboo/Graphics/SpriteMaterial.h"

namespace Bamboo
{
    struct SpriteVertexLayout
    {

        Vector3 position;
        Color color;
        ///@brief 纹理坐标
        Vector2 texCoord;
        float texIndex;
    };

    class SpriteBatch : public PrimitiveBatch<SpriteBatch, SpriteVertexLayout>
    {
    public:
        SpriteBatch() : PrimitiveBatch({.vertsPerPrimitive = 4, .indicesPerPrimitive = 6}) {}

        BufferLayout GetVertexLayout() const
        {
            return {
                {ShaderDatatType::Float3, "a_WorldPosition"},
                {ShaderDatatType::Float4, "a_Color"},
                {ShaderDatatType::Float2, "a_TexCoord"},
                {ShaderDatatType::Float, "a_TexIndex"},
            };
        }

        Ref<IndexBuffer> CreateIndexBuffer() const
        {
            const uint32_t n = m_BatchData.maxCount;
            const uint32_t ipp = m_BatchData.indicesPerPrimitive; // 每个图元的索引数（精灵 = 6）

            std::vector<uint32_t> indices(n * ipp);
            uint32_t offset = 0;

            for (uint32_t i = 0; i < n * ipp; i += ipp)
            {
                indices[i + 0] = offset + 0;
                indices[i + 1] = offset + 1;
                indices[i + 2] = offset + 2;

                indices[i + 3] = offset + 2;
                indices[i + 4] = offset + 3;
                indices[i + 5] = offset + 0;

                offset += m_BatchData.vertsPerPrimitive;
            }
            return IndexBuffer::Create(indices.data(), (uint32_t)indices.size());
        }

        Ref<Shader> GetShader() const
        {
            return Shader::Create("Quad",
                                  "BambooAssets/Shaders/sprite.vert",
                                  "BambooAssets/Shaders/sprite.frag");
        }

        void Submit(const Matrix4 &transform, const Color &color, const std::array<Vector2, 4> &uv, float textureId = 0.0f)
        {
            SpriteVertexLayout *v = AllocVertex();
            for (int i = 0; i < 4; ++i)
            {
                v[i].position = transform * kBasePositions[i];
                v[i].color = color;
                v[i].texCoord = uv[i];
                v[i].texIndex = textureId;
            }
            Commit();
        }

    private:
        static constexpr Vector3 kBasePositions[] = {
            {-0.5f, -0.5f, 0.0f},
            {0.5f, -0.5f, 0.0f},
            {0.5f, 0.5f, 0.0f},
            {-0.5f, 0.5f, 0.0f}
        };
    };
};
