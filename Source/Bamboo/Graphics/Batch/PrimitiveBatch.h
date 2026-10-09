#pragma once
#include <cstdint>
#include <vector>
#include "VertexArray.h"
#include "../Bamboo/Graphics/VertexArray.h"
#include "Shader.h"
#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/Graphics/RendererCommand.h"

namespace Bamboo
{
    struct PrimitiveData
    {
        uint32_t vertsPerPrimitive{0};
        uint32_t indicesPerPrimitive{0};
        uint32_t maxCount{0};
    };

    struct BatchStatistics
    {
        uint32_t DrawCalls{0};
        uint32_t PrimitiveCount{0};
    };

    template <typename TDerived, typename TVertex>
    class PrimitiveBatch
    {
    public:
        explicit PrimitiveBatch(const PrimitiveData &data) : m_BatchData(data) {}

        void Init(uint32_t maxCount)
        {
            m_BatchData.maxCount = maxCount;
            m_Vertices.resize(maxCount * m_BatchData.vertsPerPrimitive);

            auto *self = static_cast<TDerived *>(this);

            m_VAO = VertexArray::Create();
            m_VBO = VertexBuffer::Create(
                maxCount * m_BatchData.vertsPerPrimitive * sizeof(TVertex));
            m_VBO->SetLayout(self->GetVertexLayout());
            m_VAO->AddVertexBuffer(m_VBO);
            m_VAO->SetIndexBuffer(self->CreateIndexBuffer());
            m_Shader = self->GetShader();
        }

        void BeginBatch()
        {
            m_WrittenVertices = 0;
            m_IndexCount = 0;
            m_CurrentBatchCount = 0;
        }

        void Flush(BatchStatistics &stats)
        {
            if (m_IndexCount == 0)
                return;

            m_VBO->SetData(m_Vertices.data(),
                           m_WrittenVertices * sizeof(TVertex));
            m_VAO->Bind();
            m_Shader->Bind();
            RendererCommand::DrawIndexed(m_VAO, m_IndexCount);

            stats.DrawCalls++;
            stats.PrimitiveCount += m_IndexCount / m_BatchData.indicesPerPrimitive;
        }

        void ShutDown()
        {
            m_VAO = nullptr;
            m_VBO = nullptr;
            m_Shader = nullptr;
            m_Vertices.clear();
            m_Vertices.shrink_to_fit();
            m_WrittenVertices = 0;
            m_IndexCount = 0;
            m_CurrentBatchCount = 0;
        }

        bool IsEmpty() const { return m_IndexCount == 0; }
        bool IsFull() const { return m_CurrentBatchCount >= m_BatchData.maxCount; }
        uint32_t PrimitiveCount() const
        {
            return m_IndexCount / m_BatchData.indicesPerPrimitive;
        }

    protected:
        PrimitiveData m_BatchData;

        TVertex *AllocVertex() { return m_Vertices.data() + m_WrittenVertices; }

        void Commit()
        {
            m_WrittenVertices += m_BatchData.vertsPerPrimitive;
            m_IndexCount += m_BatchData.indicesPerPrimitive;
            m_CurrentBatchCount += 1;
        }

    private:
        std::vector<TVertex> m_Vertices;
        Ref<VertexArray> m_VAO;
        Ref<VertexBuffer> m_VBO;
        Ref<Shader> m_Shader;

        uint32_t m_WrittenVertices{0};
        uint32_t m_IndexCount{0};
        uint32_t m_CurrentBatchCount{0};
    };
}