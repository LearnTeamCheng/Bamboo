#pragma once

#include "./Math/Vector2.h";
#include "./Math/Vector3.h";
#include "./Math/Color.h"

namespace Bamboo
{
    class Renderer2D
    {
    public:
        static void Init();

        static void BeginScene();
        static void EndScene();

        static void DrawQuad(const Vector2 &position, const Vector2 &size, const Color &color);
        static void DrawQuad(const Vector3 &position, const Vector2 &size, const Color &color);

        static void DrawCircle(const Vector2 &center, float radius, const Color &color);
        static void DrawSprite();

        static void DrawTriangle();
    };
};