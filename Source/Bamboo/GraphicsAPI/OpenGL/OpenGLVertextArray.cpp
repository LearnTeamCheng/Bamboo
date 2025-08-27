#include "OpenGLVertextArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Bamboo
{

    OpenGLVertextArray::OpenGLVertextArray()
    {
        glGenVertexArrays(1, &m_RendererID);
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

    void OpenGLVertextArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
    {
        Bind();
        vertexBuffer->Bind();
        m_VertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertextArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
    {
        Bind();
        indexBuffer->Bind();
        m_IndexBuffer = indexBuffer;
    }

}