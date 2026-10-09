#pragma once
#include <string>
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
        Application(const std::string &name);
        Application(const std::string &name,int fps);
        virtual ~Application();
        void Run();
        void Stop();
        bool IsRunning();

        Scope<SceneManager> &GetSceneManager() { return m_SceneManager; }
        Scope<AssetManager> &GetAssetManager() { return m_AssetManager; }

        static Application *GetInstance() { return s_Instance; }

        Scope<Window> &GetWindow() { return m_Window; }

        void OnEvent(Event &event);
        int GetFPS() const { return m_FPS; }
        void SetFPS(int fps);

    private:
        bool OnWindowClose(ApplicationClosedEvent &event);
        bool OnWindowResize(ApplicationResizeEvent &event);

    private:
        static Application *s_Instance;
        bool m_Minimize;
        bool m_Running;
        std::string m_Name;
        Scope<Window> m_Window;

        int m_FPS{60};
        float m_LastFrameTime{0.0f};
        float m_FrameTime{0.0f};

        Scope<SceneManager> m_SceneManager;
        Scope<AssetManager> m_AssetManager;
    };
}