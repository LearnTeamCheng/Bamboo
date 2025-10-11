#include "../Bamboo/Math/Vector2.h"
struct BallComponent
{
    /// @brief 半径
    float Radius;
    /// @brief 速度
    Vector2 Velocity;

    BallComponent() = default;
    BallComponent(float radius, const Vector2& velocity)
        : Radius(radius)
       , Velocity(velocity)
    {
    }
    BallComponent(const BallComponent&)= default;

};
