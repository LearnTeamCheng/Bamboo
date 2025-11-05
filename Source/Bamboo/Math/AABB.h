#pragma once

#include "Vector3.h"
namespace Bamboo
{
    class AABB
    {
    public:
        AABB() {}
        AABB(const Vector3& min, const Vector3& max);
        const Vector3& GetMin() const { return m_Min; }
        const Vector3& GetMax() const { return m_Max; }
        void SetMin(const Vector3 &min) { m_Min = min; }
        void SetMax(const Vector3 &max) { m_Max = max; }

        /// @brief 是否相交
        bool Intersect(const AABB &other) const;
        /// @brief 是否包含点
        bool Contains(const Vector3 &point) const;
        /// @brief 是否包含AABB
        bool Contains(const AABB &other) const;
        /// @brief 是否重叠
        bool Overlaps(const AABB &other) const;
        /// @brief 合并AABB
        void Merge(const AABB &other);


    private:
        Vector3 m_Min;
        Vector3 m_Max;
    };
}