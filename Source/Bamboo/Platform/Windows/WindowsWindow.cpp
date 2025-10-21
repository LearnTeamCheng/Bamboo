

#include "../Bamboo/Event/ApplicationEvent.h"
#include "../Bamboo/Event/KeyEvent.h"
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
        BAMBOO_CORE_INFO("~WindowsWindow");
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

        //窗口关闭
        glfwSetWindowCloseCallback(m_Window, [] (GLFWwindow * window) 
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            ApplicationClosedEvent event;
            data.m_EventCallback(event);
        });

        
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) 
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            ApplicationResizeEvent event(width,height);
            data.m_EventCallback(event);
        });

        //键盘事件
        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            switch (action) {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, false);
                    data.m_EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.m_EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    break;
                }
            }
        });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            switch (action) {
            }
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