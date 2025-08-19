#pragma once
#include "../Bamboo/Math/Color.h"
#include "../Bamboo/Graphics/Texture.h"
namespace Bamboo
{

    struct SpriteRendererComponent
    {
        Color SpriteColor = Color::White;
        Ref<Texture>  SpriteTexture; 
        
        SpriteRendererComponent()=default;
        SpriteRendererComponent(const SpriteRendererComponent&)=default;
        SpriteRendererComponent(const Color& color):SpriteColor(color)  {}

    };
}
