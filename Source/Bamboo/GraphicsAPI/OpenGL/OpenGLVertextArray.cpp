#include "OpenGLVertextArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Bamboo
{

    OpenGLVertextArray::OpenGLVertextArray()
    {
        glGenVertexArrays(1, &m_RendererID);
        // glCreateVertexArrays(1, &m_RendererID);
    }

    OpenGLVertextArray::~OpenGLVertextArray()
    {
        glDeleteVertexArrays(1, &m_RendererID);
    }

    void OpenGLVertextArray::Bind() const
    {
        glBindVertexArray(m_RendererID);
    }

    void OpenGLVertextArray::Unbind() const
    {
        glBindVertexArray(0);
    }

    void OpenGLVertextArray::AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer)
    {

        glBindVertexArray(m_RendererID);
        vertexBuffer->Bind();

        auto layout = vertexBuffer->GetLayout();

     

        for(const auto& element : layout.GetElements())
        {
            switch (element.Type)
            {
            case ShaderDatatType::Float:
            case ShaderDatatType::Float2:
            case ShaderDatatType::Float3:
            case ShaderDatatType::Float4:
                glEnableVertexAttribArray(m_VertexBufferIndex);
                glVertexAttribPointer(m_VertexBufferIndex, element.GetComponentCount(), GL_FLOAT, GL_FALSE, layout.GetStride(), (void *)element.Offset);
                m_VertexBufferIndex++;
                break;
            }
        }

        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        // glEnableVertexAttribArray(0);

        m_VertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertextArray::SetIndexBuffer(const Ref<IndexBuffer> &indexBuffer)
    {
        Bind();
        indexBuffer->Bind();
        m_IndexBuffer = indexBuffer;
    }

}