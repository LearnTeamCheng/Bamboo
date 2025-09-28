#include "OpenGLContext.h"
#include "./Core/Log.h"
#include "../Bamboo/Core/Assert.h"
#include<glad/glad.h>
#include<GLFW/glfw3.h>

namespace Bamboo {

    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
        : m_WindowHandle(windowHandle)
    {
    
    }


   void OpenGLContext::Initiaize(){

        glfwMakeContextCurrent(m_WindowHandle);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        BAMBOO_ASSESERT(status , "glad init failur");
   
    }


    void OpenGLContext::SwapBuffers(){
        glfwSwapBuffers(m_WindowHandle);
    }
}