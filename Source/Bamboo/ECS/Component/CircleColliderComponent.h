#pragma once
#include "../Bamboo/Math/Vector2.h"
namespace Bamboo
{
    ///@brief 圆形碰撞体组件
    struct CircleColliderComponent
    {
        ///@brief 半径
        float Radius;
        ///@brief 中心
        Vector2 Center;
        ///@brief 偏移量
        Vector2 Offset;
        ///@brief 是否为触发器
        bool IsTrigger;

        CircleColliderComponent() = default;
        CircleColliderComponent(const CircleColliderComponent &) = default;
        CircleColliderComponent(float radius, const Vector2 &center = Vector2::Half, const Vector2 &offset = Vector2::Zero, bool isTrigger = false)
            : Radius(radius), Center(center), Offset(offset), IsTrigger(isTrigger)
        {
        }
    };

}