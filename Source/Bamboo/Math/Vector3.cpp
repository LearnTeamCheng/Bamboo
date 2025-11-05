#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
namespace Bamboo
{
    Vector3::Vector3(const Vector2 &v) : x(v.x), y(v.y), z(0.0f) {}

    Vector3::Vector3(const Vector4 &v) : x(v.x), y(v.y), z(v.z) {}

    const Vector3 Vector3::One = Vector3(1.0, 1.0, 1.0);
    const Vector3 Vector3::Zero = Vector3(0.0,0.0,0.0);

    Vector3& Vector3::operator=(const Vector4& other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    Vector3 Vector3::Min(const Vector3 &a, const Vector3 &b){
        return Vector3(Math::Min(a.x, b.x), Math::Min(a.y, b.y), Math::Min(a.z, b.z));
    }

    Vector3 Vector3::Max(const Vector3 &a, const Vector3 &b){
        return Vector3(Math::Max(a.x, b.x), Math::Max(a.y, b.y), Math::Max(a.z, b.z));
    }
}