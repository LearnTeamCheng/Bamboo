#pragma once
#include "PrimitiveBatch.h"

namespace Bamboo
{
    struct QuadVertexLayout
    {
        Vector3 position;
        Color color;
    };
    class QuadBatch : public PrimitiveBatch<QuadBatch, QuadVertexLayout>
    {
    public:
        QuadBatch() : PrimitiveBatch({.vertsPerPrimitive = 4,
                                      .indicesPerPrimitive = 6})
        {
        }

        BufferLayout GetVertexLayout() const
        {
            return {
                {ShaderDatatType::Float3, "a_WorldPosition"},
                {ShaderDatatType::Float4, "a_Color"},
            };
        }

        Ref<IndexBuffer> CreateIndexBuffer() const
        {
            const uint32_t vpp = m_BatchData.vertsPerPrimitive;
            const uint32_t n = m_BatchData.maxCount;

            std::vector<uint32_t> indices(n * m_BatchData.indicesPerPrimitive);
            int offset = 0;
            for (uint32_t i = 0; i < n; ++i)
            {
                indices[i + 0] = offset + 0;
                indices[i + 1] = offset + 1;
                indices[i + 2] = offset + 2;

                indices[i + 3] = offset + 2;
                indices[i + 4] = offset + 3;
                indices[i + 5] = offset + 0;

                offset += 4;
            }
            return IndexBuffer::Create(indices.data(), (uint32_t)indices.size());
        }

        Ref<Shader> GetShader() const
        {
            return Shader::Create("Triangle",
                                  "BambooAssets/Shaders/triangle.vert",
                                  "BambooAssets/Shaders/triangle.frag");
        }

        void Submit(const Matrix4 &transform, const Color &color)
        {
            QuadVertexLayout *v = AllocVertex();
            for (int i = 0; i < 4; ++i)
            {
                v[i].position = kBasePositions[i];
                v[i].color = color;
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