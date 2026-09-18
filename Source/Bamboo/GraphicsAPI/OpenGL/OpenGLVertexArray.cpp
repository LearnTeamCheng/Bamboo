#include "OpenGLVertexArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Bamboo
{

    OpenGLVertexArray::OpenGLVertexArray()
    {
        glGenVertexArrays(1, &m_RendererID);
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        glDeleteVertexArrays(1, &m_RendererID);
    }

    void OpenGLVertexArray::Bind() const
    {
        glBindVertexArray(m_RendererID);
    }

    void OpenGLVertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer)
    {

        glBindVertexArray(m_RendererID);
        vertexBuffer->Bind();

        auto &layout = vertexBuffer->GetLayout();

        for (const auto &element : layout.GetElements())
        {
            switch (element.Type)
            {
            case ShaderDatatType::Bool:
            case ShaderDatatType::Int:
            case ShaderDatatType::Int2:
            case ShaderDatatType::Int3:
            case ShaderDatatType::Int4:
                glEnableVertexAttribArray(m_VertexBufferIndex);
                // 注意：第五个参数在 GL 里是"指向顶点数据起始位置的字节偏移"，
                // 必须经 uintptr_t 转换，直接 (void*)uint32_t 在 64 位下会触发 C4312 且语义错误。
                glVertexAttribIPointer(m_VertexBufferIndex, element.GetComponentCount(), GL_INT, layout.GetStride(),
                                       reinterpret_cast<const void *>(static_cast<uintptr_t>(element.Offset)));
                m_VertexBufferIndex++;
                break;
            case ShaderDatatType::Float:
            case ShaderDatatType::Float2:
            case ShaderDatatType::Float3:
            case ShaderDatatType::Float4:
                glEnableVertexAttribArray(m_VertexBufferIndex);
                glVertexAttribPointer(m_VertexBufferIndex, element.GetComponentCount(), GL_FLOAT, GL_FALSE, layout.GetStride(),
                                      reinterpret_cast<const void *>(static_cast<uintptr_t>(element.Offset)));
                m_VertexBufferIndex++;
                break;
            }
        }

        m_VertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer> &indexBuffer)
    {
        Bind();
        indexBuffer->Bind();
        m_IndexBuffer = indexBuffer;
    }

}