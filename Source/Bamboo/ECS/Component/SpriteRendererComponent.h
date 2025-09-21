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
        Vector2 Size;
        
        SpriteRendererComponent()=default;
        SpriteRendererComponent(const SpriteRendererComponent&)=default;
        SpriteRendererComponent(const Color& color):SpriteColor(color)  {}

    };
}
