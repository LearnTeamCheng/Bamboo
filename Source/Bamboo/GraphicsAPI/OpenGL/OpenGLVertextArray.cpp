#include "OpenGLVertextArray.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Bamboo
{

    OpenGLVertextArray::OpenGLVertextArray()
    {
         glGenVertexArrays(1, &m_RendererID);
        //glCreateVertexArrays(1, &m_RendererID);
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
        glBindVertexArray(m_RendererID);
        vertexBuffer->Bind();

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        m_VertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertextArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
    {
        Bind();
        indexBuffer->Bind();
        m_IndexBuffer = indexBuffer;
    }

}