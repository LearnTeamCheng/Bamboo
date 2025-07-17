#include "Rect.h"
#include "Vector2.h"
namespace Bamboo
{
    Rect::Rect(const Vector2 &position, const Vector2 &size)
    {
        x = position.x;
        y = position.y;
        width = size.x;
        height = size.y;
    }
}