#include "../Core/Time.h"

#include "../Bamboo/Graphics/Renderer.h"
#include "../Bamboo/Graphics/Renderer2D.h"
#include "../Bamboo/Core/Log.h"
#include "Application.h"

namespace Bamboo
{
    Application::Application() : m_Running(true), m_Minimize(false)
    {
        // 先在这里进行初始日志操作化操作
        Log::Init();
        
        // m_Window = CreateScope<Window>();
        m_Window = Window::Create();

        m_Window->SetEventCallback(BIND_CALLBACK_FN(Application::OnEvent));
        m_SceneManager = CreateScope<SceneManager>();

        Renderer2D::Init();
        
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

            m_SceneManager->GetActiveScene()->Update(deltaTime);

            m_Window.get()->Update();
        }
    }

    void Application::Stop()
    {
        m_Running = false;
    }

    bool Application::IsRunning()
    {
        return m_Running;
    }

    void Application::OnEvent(Event &event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<ApplicationClosedEvent>(BIND_CALLBACK_FN(Application::OnWindowClose));
    }

    bool Application::OnWindowClose(ApplicationClosedEvent &event)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(ApplicationResizeEvent &event)
    {
        if (event.GetHeight() == 0 || event.GetWidth() == 0)
        {
            // 最小化了
            m_Minimize = true;
            return false;
        }
        m_Minimize = false;
        Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());

        return false;
    }

}