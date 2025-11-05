#include "AABB.h"
namespace Bamboo
{

    AABB::AABB(const Vector3& min, const Vector3& max)
    {
        m_Min = Vector3::Min(min, max);
        m_Max = Vector3::Max(min, max);
    }

    bool AABB::Overlaps(const AABB &other) const
    {
        if (m_Max.x < other.m_Min.x || m_Min.x > other.m_Max.x)
            return false;
        if (m_Max.y < other.m_Min.y || m_Min.y > other.m_Max.y)
            return false;
        if (m_Max.z < other.m_Min.z || m_Min.z > other.m_Max.z)
            return false;
        return true;
    }

    bool AABB::Intersect(const AABB &other) const
    {
        return Overlaps(other); // 如果必须保留 Intersect
    }

    bool AABB::Contains(const Vector3 &point) const
    {
        return (point.x >= m_Min.x && point.x <= m_Max.x) &&
               (point.y >= m_Min.y && point.y <= m_Max.y) &&
               (point.z >= m_Min.z && point.z <= m_Max.z);
    }

    bool AABB::Contains(const AABB &other) const
    {
        return (m_Min.x <= other.m_Min.x && m_Max.x >= other.m_Max.x) &&
               (m_Min.y <= other.m_Min.y && m_Max.y >= other.m_Max.y) &&
               (m_Min.z <= other.m_Min.z && m_Max.z >= other.m_Max.z);
    }

    void AABB::Merge(const AABB &other)
    {
        m_Min = Vector3::Min(m_Min,other.m_Min);
        m_Max = Vector3::Max(m_Min, other.m_Max);
    }
}