/// @file 顶点
#pragma once

#include <iostream>
#include "../Bamboo/Graphics/RenderBuffer.h"

namespace Bamboo
{

    class OpenGLVertexBuffer : public VertexBuffer
    {
    public:
        OpenGLVertexBuffer(uint32_t size);
        OpenGLVertexBuffer(const void *vertices, uint32_t size);

        virtual ~OpenGLVertexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void SetData(const void *vertices, uint32_t size) override;


        virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
        virtual const BufferLayout& GetLayout() const override { return m_Layout; }

    private:
        BufferLayout m_Layout;
        uint32_t m_RendererID;
    };

    class OpenGLIndexBuffer : public IndexBuffer
    {
    public:
        OpenGLIndexBuffer(uint32_t*indices, uint32_t size);
        virtual ~OpenGLIndexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual uint32_t GetCount()const override {return m_Count;}

    private:
        uint32_t m_Count;
        uint32_t m_RendererID;
    };

}