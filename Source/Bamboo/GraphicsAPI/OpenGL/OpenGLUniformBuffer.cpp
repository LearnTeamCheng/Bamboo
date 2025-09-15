#include "OpenGLUniformBuffer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Bamboo
{
    OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
    {
        glCreateBuffers(1, &m_RendererID);
        //给缓冲区分配空间
        glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
        //绑定到指定binding
        glBindBufferBase(GL_UNIFORM_BUFFER,binding,m_RendererID);
    }

    void OpenGLUniformBuffer::SetData(const void *data, uint32_t size, uint32_t offset)
    {
        glNamedBufferSubData(m_RendererID, offset, size, data);
    }

    OpenGLUniformBuffer::~OpenGLUniformBuffer(){
        glDeleteBuffers(1, &m_RendererID);
    }
}