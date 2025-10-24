#pragma once
#include  "../BreakoutDefine.h"
struct BrickComponent
{   
    /// @brief 当前砖块生命值
    int Health;
    /// @brief 砖块类型
    BrickType Type;
    /// @brief 砖块是否被击中
    bool IsHit;
    /// @brief 砖块是否被消除
    bool IsDestroyed;
    /// @brief 砖块是否可以被消除
    bool CanBeDestroyed;

    BrickComponent() = default;
    BrickComponent(const BrickComponent& other) = default;
    BrickComponent(BrickType type, int health) : Type(type), Health(health), IsHit(false), IsDestroyed(false), CanBeDestroyed(true) {}

};
