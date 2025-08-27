#pragma once
#include "../Bamboo/Core/Ref.h"
namespace Bamboo
{

    /// @brief 索引缓冲对象
    class IndexBuffer
    {
    public:
        virtual ~IndexBuffer() = default;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        virtual uint32_t GetCount() const = 0;

        static Ref<IndexBuffer> Create(uint32_t *indices, uint32_t count);
    };

    /// @brief 顶点缓冲对象
    class VertexBuffer
    {
    public:
        virtual ~VertexBuffer() = default;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        virtual void SetData(const void *data, unsigned int size) =0;

        static Ref<VertexBuffer> Create(uint32_t count);
        static Ref<VertexBuffer> Create(uint32_t *vertices, uint32_t count);
    };
}