#pragma once
#include "../Bamboo/Math/Color.h"
#include "../Bamboo/Math/Vector2.h"
#include "../Bamboo/Graphics/Texture.h"
#include "../Bamboo/Assets/AssetHandle.h"

namespace Bamboo
{

    struct SpriteRendererComponent
    {
        Color color{Color::White};
        AssetHandle handle;
        /// @brief 大小
        Vector2 size{100.0f, 100.0f};
        /// @brief 排序层级
        int zorder{0};
        /// @brief 透明度
        float opacity{1.0f};
        /// @brief 是否可见
        bool visible{true};

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent &) = default;
        SpriteRendererComponent(const Color &color) : color(color) {}
    };
}
