#pragma once
#include "Game.h"
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
        Game m_game;
    };
}