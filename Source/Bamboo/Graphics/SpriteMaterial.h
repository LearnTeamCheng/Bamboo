#pragma once
#include <cstdint>

#include "../Bamboo/Core/Ref.h"
#include "Shader.h"

namespace Bamboo
{
    class SpriteMaterial
    {
    public:
        uint32_t textureID;
        Ref<Shader> shader;
    };
};