#pragma once
#include "Game.h"
#include <memory>
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
        std::unique_ptr<Game> m_game;
        

    };
}