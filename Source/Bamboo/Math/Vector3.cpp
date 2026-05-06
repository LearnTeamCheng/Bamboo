#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
namespace Bamboo
{
    Vector3::Vector3(const Vector2 &v) : x(v.x), y(v.y), z(0.0f) {}

    Vector3::Vector3(const Vector4 &v) : x(v.x), y(v.y), z(v.z) {}

    Vector3 &Vector3::operator=(const Vector4 &other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    Vector3 Vector3::Min(const Vector3 &a, const Vector3 &b)
    {
        return Vector3(Math::Min(a.x, b.x), Math::Min(a.y, b.y), Math::Min(a.z, b.z));
    }

    Vector3 Vector3::Max(const Vector3 &a, const Vector3 &b)
    {
        return Vector3(Math::Max(a.x, b.x), Math::Max(a.y, b.y), Math::Max(a.z, b.z));
    }

    const Vector3 Vector3::One = Vector3(1.0f, 1.0f, 1.0f);
    const Vector3 Vector3::Zero = Vector3(0.0f, 0.0f, 0.0f);
    /// @brief  后方
    const Vector3 Vector3::Backward = Vector3(0.0f, 0.0f, -1.0f);
    /// @brief  前方
    const Vector3 Vector3::Forward = Vector3(0.0f, 0.0f, 1.0f);
    const Vector3 Vector3::Left = Vector3(-1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::Right = Vector3(1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::Up = Vector3(0.0f, 1.0f, 0.0f);
    const Vector3 Vector3::Down = Vector3(0.0f, -1.0f, 0.0f);


}