#include "../Bamboo/Math/Vector2.h"
struct BallComponent
{
    /// @brief 半径
    float Radius;
    /// @brief 速度
    Bamboo::Vector2 Velocity;

    BallComponent() = default;
    BallComponent(float radius, const Bamboo::Vector2& velocity)
        : Radius(radius)
       , Velocity(velocity)
    {
    }
    BallComponent(const BallComponent&)= default;

};
