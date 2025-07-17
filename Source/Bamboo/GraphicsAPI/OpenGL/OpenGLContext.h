#pragma once

#include "../Graphics/GraphicsContext.h"

struct  GLFWwindow;

namespace Bamboo {
    class OpenGLContext : public GraphicsContext {
        public:
            OpenGLContext(GLFWwindow* windowHandle);
          virtual  void Initiaize();
           virtual void SwapBuffers();
        private:
            GLFWwindow* m_WindowHandle;

    };
}