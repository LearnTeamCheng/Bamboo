#include "OpenGLContext.h"
#include "Bamboo/Core/Log.h"
#include<GLFW/glfw3.h>
#include<glad/glad.h>

namespace Bamboo {

    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
        : m_WindowHandle(windowHandle)
    {
    
    }


   void OpenGLContext::Initiaize(){

        glfwMakeContextCurrent(m_WindowHandle);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        if(!status) {
            BAMBOO_LOGI("Failed to initialize GLAD");
        }
    }


    void OpenGLContext::SwapBuffers(){
        glfwSwapBuffers(m_WindowHandle);
    }
}