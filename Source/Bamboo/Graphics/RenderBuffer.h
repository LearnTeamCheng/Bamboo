#pragma once
#include <string>
#include <vector>
#include "../Bamboo/Core/Ref.h"
namespace Bamboo
{

    enum class ShaderDatatType
    {
        None,Float,Float2,Float3,Float4,Int,Int2,Int3,Int4,Mat3,Mat4,Bool
    };


    static uint32_t ShaderDataTypeSize(ShaderDatatType type){
        switch (type)
        {
            case ShaderDatatType::Float:    return 4;
            case ShaderDatatType::Float2:   return 4 * 2;
            case ShaderDatatType::Float3:   return 4 * 3;
            case ShaderDatatType::Float4:   return 4 * 4;
            case ShaderDatatType::Int:      return 4;
            case ShaderDatatType::Int2:     return 4 * 2;
            case ShaderDatatType::Int3:     return 4 * 3;
            case ShaderDatatType::Int4:     return 4 * 4;
            case ShaderDatatType::Mat3:     return 4 * 3 * 3;
            case ShaderDatatType::Mat4:     return 4 * 4 * 4;
            case ShaderDatatType::Bool:     return 1;
        }
        return 0;
    }


    struct BufferElement
    {
        std::string Name;
        ShaderDatatType Type;
        uint32_t Size;
        uint32_t Offset;
        bool Normalized;

        BufferElement()=default;
        BufferElement(ShaderDatatType type, const std::string& name, bool normalized = false)
            : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) {
        }

        uint32_t GetComponentCount() const
        {
           switch (Type)
           {
               case ShaderDatatType::Float:    return 1;
               case ShaderDatatType::Float2:   return 2;
               case ShaderDatatType::Float3:   return 3;
               case ShaderDatatType::Float4:   return 4;
               case ShaderDatatType::Int:      return 1;
               case ShaderDatatType::Int2:     return 2;
               case ShaderDatatType::Int3:     return 3;
               case ShaderDatatType::Int4:     return 4;
               case ShaderDatatType::Mat3:     return 3;
               case ShaderDatatType::Mat4:     return 4;
               case ShaderDatatType::Bool:     return 1;
               default:
                   return 0;
           }

        }
    };

    class BufferLayout
    {
        public:
            BufferLayout() = default;
            BufferLayout(const std::initializer_list<BufferElement>& elements):m_Elements(elements)
            {
                CalculateOffsetsAndStride();
            }

            uint32_t GetStride() const {return m_Stride;}

            const std::vector<BufferElement> GetElements() const {return m_Elements;}

            std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
            std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
            std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
            std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

        private:
            void CalculateOffsetsAndStride(){
                size_t offset = 0;
                for (auto& element : m_Elements)
                {
                    element.Offset = offset;
                    offset += element.Size;
                    m_Stride += element.Size;
                }
            }

            std::vector<BufferElement> m_Elements;
            /// @brief 计算布局的字节大小
            uint32_t m_Stride = 0;
    };
    

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
        virtual void SetData(const void *data, unsigned int size) = 0;
        virtual const BufferLayout& GetLayout() const = 0;
        virtual void SetLayout(const BufferLayout& layout) = 0;

        static Ref<VertexBuffer> Create(uint32_t count);
        static Ref<VertexBuffer> Create(uint32_t *vertices, uint32_t count);
    };
}