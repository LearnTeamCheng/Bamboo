#include "OpenGLContext.h"

#include<GLFW/glfw3.h>
#include<glad/glad.h>

namespace Bamboo {

    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
        : m_WindowHandle(windowHandle)
    {
    
    }


   void OpenGLContext::Initiaize(){

        glfwMakeContextCurrent(m_WindowHandle);
    }


    void OpenGLContext::SwapBuffers(){
        glfwSwapBuffers(m_WindowHandle);
    }
}