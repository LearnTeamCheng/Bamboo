#include "Vector2.h"
#include "Vector3.h"

namespace Bamboo
{
    Vector3::Vector3(const Vector2 &v) : x(v.x), y(v.y), z(1.0f) {}

    const Vector3 Vector3::One = Vector3(1.0, 1.0, 1.0);
    const Vector3 Vector3::Zero = Vector3(0.0,0.0,0.0);
}