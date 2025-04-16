#include "Application.h"

namespace Bamboo
{
    Application::Application()
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
        return true;
    }

    void Application::Stop(){
        
    }

}