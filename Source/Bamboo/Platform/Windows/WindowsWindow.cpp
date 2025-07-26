#include "WindowsWindow.h"
namespace Bamboo
{

    WindowsWindow::WindowsWindow(const WindowProps&props)
    {
        Initilize(props);
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

        if (!glfwInit())
        {
            BB_CORE_ERROR("Failed to initialize GLFW");
            return;
        }

        m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
        if (!m_Window)
        {
            BB_CORE_ERROR("Failed to create GLFW window");
            glfwTerminate();
            return;
        }
        
    }

    void WindowsWindow::Update() {
        glfwPollEvents();
        m_Context->SwapBuffers();
    }

    void WindowsWindow::Shutdown(){
        glfwTerminate();
    }

}