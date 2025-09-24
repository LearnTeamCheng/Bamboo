#pragma once

#include "./Math/Vector2.h"
#include "./Math/Vector3.h"
#include "./Math/Matrix4.h"
#include "./Math/Color.h"
#include "../Bamboo/Graphics/Texture.h"
#include <string>

namespace Bamboo
{
    class Camera;
    class Renderer2D
    {
    public:
        static void Init();

        static void BeginScene();
        static void BeginScene(const Camera& camera);
        static void EndScene();
        static void Shutdown();

        static void DrawQuad(const Vector2 &position, const Vector2 &size, const Color &color);
        static void DrawQuad(const Vector3 &position, const Vector2 &size, const Color &color);

        static void DrawCircle(const Vector2 &center, float radius, const Color &color);
        static void DrawSprite(const Matrix4& localMatrix, const Color & color, Ref<Texture2D>& texture);

        static void DrawTriangle(const Vector3& position, const Color& color);

    private:
        static void StartBatch();
    };
};