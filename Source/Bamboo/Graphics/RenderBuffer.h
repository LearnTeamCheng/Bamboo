#pragma once

namespace Bamboo
{

    /// @brief 索引缓冲对象
    class IndexBuffer
    {
    public:
        virtual ~IndexBuffer() = default;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
         virtual void GetCount()const = 0

       

    };

    /// @brief 顶点缓冲对象
    class VertexBuffer
    {
        public:
            virtual ~VertexBuffer() = default;
            virtual void Bind() const = 0;
            virtual void Unbind() const = 0;
            virtual void SetData(const void *data, unsigned int size);

    };
}