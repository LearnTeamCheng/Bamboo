/**
 * @file Vector4.h
 * @brief vector 4
 */
#pragma once

#include "MathDefs.h"
#include "Vector3.h"
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

    public:
        Vector4(float x, float y, float z, float w) :x(x), y(y), z(z), w(w) {};
        Vector4() :x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        Vector4(const Vector4& other) = default;

        Vector4(const Vector3& other) :x(other.x), y(other.y), z(other.z), w(1.0) {}
    };
}