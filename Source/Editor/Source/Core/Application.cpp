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

        // ⚠️ 已知缺陷：这里没有初始化 GL3 后端（缺少 ImGui_ImplOpenGL3_Init），
        // 所以 ImGui 的绘制数据不会被提交到 OpenGL，窗口里什么都看不到。
        // 另外整个编辑器绕过了引擎（自己 glfwInit/建窗），见 refactor_plan.md §4.5。
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, false);

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // TODO(编辑器): 面板绘制写在这里（后续会拆成 EditorLayer 的各个 Panel）

            ImGui::Render();
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