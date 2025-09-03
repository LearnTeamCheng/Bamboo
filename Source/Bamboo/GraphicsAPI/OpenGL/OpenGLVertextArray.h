#pragma once
#include <iostream>
#include "../Bamboo/Graphics/VertexArray.h"

namespace Bamboo
{

    class OpenGLVertextArray : public VertexArray
    {
    public:
        OpenGLVertextArray();
        virtual ~OpenGLVertextArray();
        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
        virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

        const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override {return m_VertexBuffers;}
        const Ref<IndexBuffer>& GetIndexBuffer() const override {return m_IndexBuffer;}

    private:
        uint32_t m_RendererID;
        uint32_t m_VertexBufferIndex = 0;

        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        Ref<IndexBuffer> m_IndexBuffer;
    };
}