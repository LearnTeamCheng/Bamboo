#include "Vector2.h"
#include "Vector3.h"

namespace Bamboo
{
    Vector3::Vector3(const Vector2 &v) : x(v.x), y(v.y), z(1.0f) {}
}