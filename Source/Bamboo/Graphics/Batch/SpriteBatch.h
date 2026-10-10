#pragma once
#include "PrimitiveBatch.h"

#include <unordered_map>
namespace Bamboo
{
    struct SpriteVetexLayout
    {

        Vector3 position;
        Color color;
        ///@brief 纹理坐标
        Vector2 texCoord;
        float texIndex;
    };

    class SpriteBatch : public PrimitiveBatch<SpriteBatch, SpriteVetexLayout>
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

            // 注意：i 遍历的是**索引数组的下标**，边界必须是"索引总数 n * ipp"，
            // 不能写成 i < n（n 是图元数），否则循环只跑 1/6 次，其余索引保持为 0。
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

        void Submit(const Matrix4 &transform, const Color &color)
        {
            SpriteVetexLayout *v = AllocVertex();
            for (int i = 0; i < 4; ++i)
            {
                v[i].position = transform * kBasePositions[i];
                v[i].color = color;
                v[i].texCoord = kTextCoord[i];
                // 必须显式赋值：槽位 0 是白纹理，不写就是未初始化数据
                v[i].texIndex = kDefaultTextureIndex;
            }
            Commit();
        }

    private:
        // std::unordered_map<int,>
    private:
        static constexpr Vector3 kBasePositions[] = {
            {-0.5f, -0.5f, 0.0f},
            {0.5f, -0.5f, 0.0f},
            {0.5f, 0.5f, 0.0f},
            {-0.5f, 0.5f, 0.0f}

        };

        static constexpr Vector2 kTextCoord[] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f},
        };

        /// 纹理槽 0 固定是白纹理（见 Renderer2D 的 StartBatch）
        static constexpr float kDefaultTextureIndex = 0.0f;
    };
};
