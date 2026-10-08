#pragma once
// #include <con
#include "PrimitiveBatch.h"
#include <utility>
#include <vector>

namespace Bamboo
{
    struct TriangleVertex
    {
        Vector3 position;
        Color color;
    };

    class TriangleBatch : public PrimitiveBatch<TriangleBatch, TriangleVertex>
    {
    public:
        TriangleBatch()
            : PrimitiveBatch({
                  .vertsPerPrimitive = 3,
                  .indicesPerPrimitive = 3,
              })
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

            std::vector<uint32_t> indices(n * 3);
            for (uint32_t i = 0; i < n; ++i)
            {
                indices[i * 3 + 0] = i * vpp + 0;
                indices[i * 3 + 1] = i * vpp + 1;
                indices[i * 3 + 2] = i * vpp + 2;
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
            TriangleVertex *v = AllocVertex();
            for (int i = 0; i < 3; ++i)
            {
                v[i].position = transform * kBasePositions[i];
                v[i].color = color;
            }
            Commit();
        }

    private:
        static constexpr Vector3 kBasePositions[3] = {
            {-0.5f, -0.5f, 0.f},
            {0.5f, -0.5f, 0.f},
            {0.0f, 0.5f, 0.f},
        };
    };
}