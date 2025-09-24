
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "./GraphicsAPI/OpenGL/OpenGLRendererAPI.h"

namespace Bamboo
{
    void OpenGLRendererAPI::Init()
    {   
        //线框模式
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //深度测试
        glEnable(GL_DEPTH_TEST);
        //混合
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height){
        glViewport(x, y, width, height);
        
    }

    void OpenGLRendererAPI::SetClearColor(const Color& color){
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void OpenGLRendererAPI::Clear(){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }


    void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount){
        vertexArray->Bind();
        uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    }


}

