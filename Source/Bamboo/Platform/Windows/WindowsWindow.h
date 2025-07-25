#pragma once

#include <memory>
#include <string>
#include <functional>

#include "../../Core/Log.h"
#include "../../Game/Window.h"
#include "../../Graphics/GraphicsContext.h"

#define GLFW_INCLUDE_NONE
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
        virtual void Initialize(const WindowProps& props);
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