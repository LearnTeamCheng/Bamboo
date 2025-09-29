#include "Application.h"
#include "../Bamboo/Core/Log.h"

namespace BambooEditor
{

    Application::Application() : m_IsRunning(false)
    {
    }

    bool Application::Initialize()
    {
        // todo: 初始化相关操作
        return false;
    }

    void Application::Update()
    {
        // todo: 更新相关操作
    }

    void Application::Renderer()
    {
        // todo: 渲染相关操作
    }

    void Application::Run()
    {
        if (!Initialize())
        {
            BAMBOO_CORE_ERROR("Failed to initialize application");
            return;
        }

        while (m_IsRunning)
        {
            // 事件
            Update();
            Renderer();
        }
    }

    void Application::Shoudown()
    {
        // todo: 关闭程序相关操作
    }

    Application &Application::GetInstance()
    {
        static Application app;
        return app;
    }

}