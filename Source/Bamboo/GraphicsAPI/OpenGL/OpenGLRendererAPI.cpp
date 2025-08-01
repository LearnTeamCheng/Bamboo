#include "Bamboo/GraphicsAPI/OpenGL/OpenGLRendererAPI.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Bamboo
{
    void OpenGLRendererAPI::Init()
    {
    }

    void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height){
        glViewport(x, y, width, height);
    }

    void OpenGLRendererAPI::SetClearColor(const Color& color){
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void OpenGLRendererAPI:Clear(){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

