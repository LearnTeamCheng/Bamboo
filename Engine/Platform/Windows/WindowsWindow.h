#pragma once
#include "../../Game/Window.h"
#include <GLFW/glfw3.h>
namespace Bamboo
{

    class WindowsWindow : public Window
    {

    public:
        WindowsWindow(const WindowProps& props);
        virtual ~WindowsWindow();

        void Update() override;

        unsigned int GetWidth() const override { return m_Data.Width; }
        unsigned int GetHeight() const override { return m_Data.Height; }
    private:
        virtual void Initilize(const WindowProps& props);
        virtual void Shutdown();

    private:
        GLFWwindow* m_Window;
        Scope<GraphicsContext> m_Context;

        struct WindowData
        {
            std::string Title;
            unsigned int Width, Height;
        };

        WindowData m_Data;
    };
};