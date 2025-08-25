#pragma once
#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/Graphics/RendererAPI.h"
#include "../Bamboo/Math/Color.h"

namespace Bamboo
{
    class RendererCommand
    {
    public:
        static void  SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            s_RendererAPI->SetViewport(x, y, width, height);
        }

        static void SetClearColor(const Color& color)
        {
            s_RendererAPI->SetClearColor(color);
        }

        static void Clear()
        {
            s_RendererAPI->Clear();
        }

    private:
        static Scope<RendererAPI> s_RendererAPI;
    };
};