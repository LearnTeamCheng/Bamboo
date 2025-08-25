#pragma once

#include "./Math/Vector2.h";
#include "./Math/Vector3.h";
#include "./Math/Color.h"

namespace Bamboo
{
    class Renderer2d
    {
    public:
        static void DrawQuad(const Vector2 &position, const Vector2 &size, const Color &color);
        static void DrawQuad(const Vector3 &position, const Vector2 &size, const Color &color);

        static void DrawCircle(const Vector2 &center, float radius, const Color &color);
        static void DrawSprite();
    };
};