#pragma once


#include "../Bamboo/Core/KeyCodes.h";
#include "../Bamboo/Math/Vector2.h";


namespace Bamboo
{
    class Input
    {
    public:
        static bool IsKeyPressed(KeyCode key);
        static Vector2 GetMousePosition();
    };
}