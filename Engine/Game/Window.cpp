#include "Window.h"
#include "../Platform/Windows/WindowsWindow.h"

namespace Bamboo
{
    Scope<Window> Window::Create(const WindowProps& props){
        return  CreateScope<WindowsWindow>(props);
    }
}