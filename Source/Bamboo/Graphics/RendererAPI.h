#pragma once
#include <cstdint>
#include "Bamboo/Core/Ref.h"
#include "Bamboo/Math/Color.h"
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

        virtual void  SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

        virtual void  ClearColor(const Color & color) = 0;
        virtual void  Clear() = 0;


        static API GetAPI() { return s_API; }
        static Scope<RendererAPI> Create();

    private:
        static API s_API;
    };
}