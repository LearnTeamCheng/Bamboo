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
        Game m_game;
    };
}