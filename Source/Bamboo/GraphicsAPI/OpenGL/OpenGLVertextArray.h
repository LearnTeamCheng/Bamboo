#pragma once
#include <iostream>
#include "../Bamboo/Graphics/VertexArray.h"

namespace bamboo
{

    class OpenGLVertextArray : public VertexArray
    {
        OpenGLVertextArray();
        virtual ~OpenGLVertextArray();
        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
        virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

        const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override {return m_VertexBuffers;}
        const Ref<IndexBuffer>& GetIndexBuffer() const override {return m_IndexBuffer;}

    private:
        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        u_int32_t m_RendererID;
        Ref<IndexBuffer> m_IndexBuffer;
    };
}