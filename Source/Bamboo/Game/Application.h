#pragma once
#include "Window.h"
#include "../Bamboo/Core/Base.h"
#include "../Bamboo/Event/Event.h"
#include "../Bamboo/Event/ApplicationEvent.h";
#include "../Core/Ref.h"

namespace Bamboo
{
    class Application
    {
    public:
        Application();
        virtual ~Application();
        void Run();
        void Stop();
        bool IsRunning();
    
    private:
        void OnEvent(Event& event);
        bool OnWindowClose(ApplicationClosedEvent& event);
        bool OnWindowResize(ApplicationResizeEvent& event);
    private:
        bool m_Minimize;
        bool m_Running;
        Scope<Window> m_window;

    };
}