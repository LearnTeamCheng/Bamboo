#pragma once

#include "../Core/Ref.h"

#include <sstream>
namespace Bamboo
{
    /// @brief 窗口属性结构体
    struct  WindowProps
    {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowProps(const std::string& title = "Bamboo Engine",
                    uint32_t width = 1280,
                    uint32_t height = 720)
            : Title(title), Width(width), Height(height)
        {
        }
    };
    

    class Window
    {
    private:
        /* data */
    public:
       virtual ~Window() = default;
       virtual void Update() = 0;

       virtual uint32_t GetWidth() const = 0;
       virtual uint32_t GetHeight() const = 0;


       static Scope<Window> Create(const WindowProps& props = WindowProps());
    };

}