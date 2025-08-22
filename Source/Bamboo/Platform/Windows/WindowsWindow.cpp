

#include "../Bamboo/Event/ApplicationEvent.h"
#include "../Bamboo/Core/Log.h"
#include "WindowsWindow.h"

namespace Bamboo
{

    WindowsWindow::WindowsWindow(const WindowProps&props)
    {
        Initialize(props);
    }

    WindowsWindow::~WindowsWindow()
    {
        Shutdown();
    }

    void WindowsWindow::Initialize(const WindowProps& props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        BAMBOO_CORE_INFO("Creating window {0} ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height);

        if (!glfwInit())
        {
            //BB_CORE_ERROR("Failed to initialize GLFW");
            return;
        }

        m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
        if (!m_Window)
        {
            //BB_CORE_ERROR("Failed to create GLFW window");
            glfwTerminate();
            return;
        }
        
        m_Context = GraphicsContext::Create(m_Window);
        m_Context->Initiaize();

        glfwSetWindowUserPointer(m_Window,&m_Data);

        //
        glfwSetWindowCloseCallback(m_Window, [] (GLFWwindow * window) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            ApplicationClosedEvent event;
            data.m_EventCallback(event);
               
            });

        
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
               WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
               ApplicationResizeEvent event(width,height);
               data.m_EventCallback(event);

            });
    }

    void WindowsWindow::Update() {
        glfwPollEvents();
        m_Context->SwapBuffers();
    }

    void WindowsWindow::SetEventCallback(const EventCallbackFn& callback) 
    {
        m_Data.m_EventCallback = callback;
    }

    void WindowsWindow::Shutdown(){
        glfwTerminate();
    }

}