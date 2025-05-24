#pragma once
#include "../Core/Ref.h"
#include "Window.h"

namespace Bamboo
{
    class Application
    {
    public:
        Application();
        ~Application();
        void Run();
        void Stop();
        bool IsRunning();

    private:
        bool m_isRunning;
        //std::unique_ptr<Game> m_game;
        Scope<Window> m_window;
        

    };
}