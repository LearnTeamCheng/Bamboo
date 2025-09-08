#include "Vector3.h"

#include "Vector2.h"
namespace Bamboo
{

    Vector2::Vector2(const Vector3& v) :x(v.x), y(v.y) {}

    const Vector2 Vector2::Zero = Vector2(0.0f, 0.0f);
    const Vector2 Vector2::One = Vector2(1.0f, 1.0f);
    const Vector2 Vector2::Up = Vector2(0.0f, 1.0f);
    const Vector2 Vector2::Down = Vector2(0.0f, -1.0f);
    const Vector2 Vector2::Left = Vector2(-1.0f, 0.0f);
    const Vector2 Vector2::Right = Vector2(1.0f, 0.0f);
}
