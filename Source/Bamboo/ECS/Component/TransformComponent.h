#pragma once

#include "../../Math/Vector3.h"
#include "../../Math/Matrix4.h"
namespace Bamboo
{
    struct TransformComponent
    {
        Vector3 position{0.0f, 0.0f, 0.0f};
        Vector3 rotation{0.0f, 0.0f, 0.0f};
        Vector3 scale{1.0f, 1.0f, 1.0f};

        bool dirty = true;
        Matrix4 localMatrix;
        Matrix4 worldMatrix;

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

            localMatrix(0, 3) = position.x;
            localMatrix(1, 3) = position.y;
            localMatrix(2, 3) = position.z;

            localMatrix(0, 0) = scale.x;
            localMatrix(1, 1) = scale.y;
            localMatrix(2, 2) = scale.z;
        }

        void SetPosition(Vector3 &position);
        const Vector3 &GetPosition() const;

        void SetRotation(Vector3 &rotation);
        const Vector3 &GetRotation() const;

        const Vector3 &GetScale() const;

        void Translate(Vector3 &translation);
        void Rotate(Vector3 &rotation);
        void Scale(Vector3 &scale);


    private:
        uint64_t m_LocalVersion{0};
        uint64_t m_WorldVersion{0}; // 世界版本
    };

}