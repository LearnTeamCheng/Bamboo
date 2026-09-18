#include "../Bamboo/Graphics/RendererAPI.h"
#include "../Bamboo/Graphics/RendererCommand.h"
#include "Renderer.h"
#include "./Renderer2D.h"


namespace Bamboo
{

    void Renderer::Init()
    {
        RendererCommand::Init();
    }

    void Renderer::Shutdown()
    {
        Renderer2D::Shutdown();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        // TODO(渲染): 目前是空实现 —— 窗口缩放时不会更新视口，相机的宽高比也保持不变，
        // 所以画面会被拉伸（见 refactor_plan.md P2-9）。
        // 应在这里调用 RendererCommand::SetViewport(0, 0, width, height)，
        // 并把新的尺寸通知给当前相机（Camera::SetViewportSize）。
        (void)width;
        (void)height;
    }

} // namespace Bamboo
