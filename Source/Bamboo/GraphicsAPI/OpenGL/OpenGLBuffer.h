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

    private:
        uint32_t m_RendererID;
    };

    class OpenGLIndexBuffer : public IndexBuffer
    {
    public:
        OpenGLIndexBuffer(const void *indices, uint32_t size);
        virtual ~OpenGLIndexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void GetCount()const override {return m_Count;}

    private:
        unit32_t m_Count;
        uint32_t m_RendererID;
    };

}