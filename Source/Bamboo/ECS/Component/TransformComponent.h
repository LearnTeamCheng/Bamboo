#pragma once

#include "../../Math/Vector3.h"
#include "../../Math/Matrix4.h"
namespace Bamboo
{
    struct TransformComponent
    {
        Vector3 Position{0.0f, 0.0f, 0.0f};
        Vector3 Rotation{0.0f, 0.0f, 0.0f};
        Vector3 Scale{1.0f, 1.0f, 1.0f};

        bool Dirty = true;
        Matrix4 LocalToWorldMatrix;
        Matrix4 WorldToLocalMatrix;

        TransformComponent() = default;
        TransformComponent(const TransformComponent &other) = default;

        void SetLocalToWorldMatrix(Vector3 &position, Vector3 &scale)
        {
            // 设置 矩阵 位置 缩放
            /** sx,0,0,x
             *  0,sy,0,y
             *  0,0,sz,z
             *  0,0,0,1
             */

            LocalToWorldMatrix(0, 3) = position.x;
            LocalToWorldMatrix(1, 3) = position.y;
            LocalToWorldMatrix(2, 3) = position.z;

            LocalToWorldMatrix(0, 0) = scale.x;
            LocalToWorldMatrix(1, 1) = scale.y;
            LocalToWorldMatrix(2, 2) = scale.z;
        }
    };

}