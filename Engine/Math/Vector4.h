/**
 * @file Vector4.h
 * @brief vector 4
 */
#pragma once

#include "MathDefs.h"
namespace Bamboo
{
    class Vector4
    {
    public:
        union
        {
            struct
            {
                float x, y, z, w;
            };
            float data[4];
        };
    };
};