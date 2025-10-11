#pragma once


#include "../Bamboo/Core/KeyCodes.h";
#include "../Bamboo/Math/Vector2.h";


namespace Bamboo
{
    class Input
    {
    public:
        /// @brief 检查是否按下某个键
        /// @param key 要检查的键的KeyCode
        static bool IsKeyPressed(KeyCode key);
        /// @brief 获取鼠标位置
        static Vector2 GetMousePosition();
    };
}