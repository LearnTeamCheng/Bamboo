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
        //Vector3 result{other.x,other.y,other.z};
        //return result;

        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }
}