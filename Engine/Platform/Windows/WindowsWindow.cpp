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

    void WindowsWindow::Initilize(const WindowProps&props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        
    }

    void WindowsWindow::Update() {
        
    }

    void WindowsWindow::Shutdown(){

    }

}