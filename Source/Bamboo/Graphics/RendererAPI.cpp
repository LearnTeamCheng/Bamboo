#include "./Graphics/RendererAPI.h"

#include "./GraphicsAPI/OpenGL/OpenGLRendererAPI.h"

namespace Bamboo
{
    RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

    Scope<RendererAPI> RendererAPI::Create()
    {
        // TODO(渲染): 目前无论 s_API 是什么都返回 OpenGL 实现 —— 抽象层的"策略"部分被架空了。
        // 应该恢复按 s_API 分发的 switch，或在决定"只支持 OpenGL"后删掉这层抽象。
        // 见 P2-9 与 D1。
        return CreateScope<OpenGLRendererAPI>();
    }
}