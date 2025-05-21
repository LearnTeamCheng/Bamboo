#include "Application.h"

namespace Bamboo
{
    Application::Application():
        m_isRunning(true)
    {
    }

    Application::~Application()
    {
    }

    void Application::Run()
    {
        while (IsRunning())
        {
            m_game.Update();
        }
    }

    bool Application::IsRunning()
    {
        return m_isRunning;
    }

    void Application::Stop(){
        m_isRunning = false;
    }

}