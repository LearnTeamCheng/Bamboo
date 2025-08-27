#pragma once
#include <cstdint>
#include "./Core/Ref.h"
#include "./Math/Color.h"
#include "../Bamboo/Graphics/VertexArray.h"
namespace Bamboo
{

    class RendererAPI
    {
    public:
        enum class API
        {
            None = 0,
            OpenGL = 1,
        };

    public:
        ~RendererAPI() = default;

        virtual void Init();
        virtual void Shutdown();

        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

        virtual void SetClearColor(const Color &color) = 0;
        virtual void Clear() = 0;

        virtual void DrawIndexed(const Ref<VertexArray> &vertexArray, uint32_t indexCount = 0) = 0;
        static API GetAPI() { return s_API; }
        static Scope<RendererAPI> Create();

    private:
        static API s_API;
    };
}