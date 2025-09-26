#pragma once

#include "Vector3.h"
namespace Bamboo
{
    class AABB
    {
    public:
        AABB() {}
        AABB(const Vector3 &min, const Vector3 &max) : m_Min(min), m_Max(max) {}

    private:
        Vector3 m_Min;
        Vector3 m_Max;
    };
}