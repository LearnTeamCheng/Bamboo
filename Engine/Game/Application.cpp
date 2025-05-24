
#include "../Core/Time.h"

#include "Application.h"

namespace Bamboo
{
    Application::Application() :
        m_isRunning(true)
    {
        //m_window = CreateScope<Window>();
        m_window = Window::Create();
    }

    Application::~Application()
    {
    }

    void Application::Run()
    {
        Time::Initialize();

        while (IsRunning())
        {
            Time::Update();
            float deltaTime = Time::GetDeltaTime();
            
            m_window.get()->Update();

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