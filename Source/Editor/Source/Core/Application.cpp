#include "Application.h"
#include "../Bamboo/Core/Log.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <GLFW/glfw3.h>
namespace BambooEditor
{

    Application::Application() : m_IsRunning(true)
    {
        Initialize();
    }

    bool Application::Initialize()
    {

        glfwInit();
        // todo: 初始化相关操作
        return true;
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
        GLFWwindow* window = glfwCreateWindow(800,600,"Editor",nullptr,nullptr);

      

        if (!Initialize())
        {
            BAMBOO_CORE_ERROR("Failed to initialize application");
            return;
        }

        //while (m_IsRunning)
        //{
        //    // 事件
        //    Update();
        //    Renderer();

        // 

        //    glfwPollEvents();
        //    glfwSwapBuffers(window);
        //    ImGui::Render();
        //}

        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, false);

        while (!glfwWindowShouldClose(window))
        {
            //一些注释...
            glfwPollEvents();
            // Start the Dear ImGui frame
          
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            /*
            * 添加自己的代码,App的实现见下面的代码
            */
            //App::RenderUI();

            // Rendering
            ImGui::Render();
            //一些代码和注释
            glfwSwapBuffers(window);
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