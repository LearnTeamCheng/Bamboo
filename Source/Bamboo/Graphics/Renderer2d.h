#pragma once

#include "./Math/Vector2.h";
#include "./Math/Vector3.h";
#include "./Math/Color.h"

namespace Bamboo
{
    class Renderer2d
    {
    public:
        static void DrawQuad();
        static void DrawCircle(const Vector2& center, float radius, const Color& color);
        static void DrawSprite();
    };
};