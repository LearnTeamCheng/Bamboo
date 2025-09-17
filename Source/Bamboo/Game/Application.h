#pragma once
#include "Window.h"
#include "../Bamboo/Core/Base.h"
#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/Event/Event.h"
#include "../Bamboo/Event/ApplicationEvent.h"

#include "../Bamboo/Scene/SceneManager.h"

#include "../Bamboo/Assets/AssetManager.h"

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
        
        Scope<SceneManager>& GetSceneManager() { return  m_SceneManager; }
        Scope<AssetManager>& GetAssetManager() { return m_AssetManager; }

        static Application* GetInstance() { return s_Instance; }

    private:
        void OnEvent(Event& event);
        bool OnWindowClose(ApplicationClosedEvent& event);
        bool OnWindowResize(ApplicationResizeEvent& event);
    private:
        static Application* s_Instance;
        bool m_Minimize;
        bool m_Running;
        Scope<Window> m_Window;

        Scope<SceneManager> m_SceneManager;
        Scope<AssetManager> m_AssetManager;
    };
}