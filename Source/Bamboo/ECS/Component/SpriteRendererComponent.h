#pragma once
#include "../Bamboo/Math/Color.h"
#include "../Bamboo/Math/Vector2.h"
#include "../Bamboo/Graphics/Texture.h"
namespace Bamboo
{

    struct SpriteRendererComponent
    {
        Color SpriteColor = Color::White;
        Ref<Texture2D>  SpriteTexture; 
        /// @brief 大小
        Vector2 Size = {100.0f, 100.0f};
        /// @brief 排序层级
        int ZOrder = 0;
        /// @brief 透明度
        float Opacity = 1.0f;
        /// @brief 是否可见
        bool Visible = true;
     

        SpriteRendererComponent()=default;
        SpriteRendererComponent(const SpriteRendererComponent&)=default;
        SpriteRendererComponent(const Color& color):SpriteColor(color)  {}

    };
}
