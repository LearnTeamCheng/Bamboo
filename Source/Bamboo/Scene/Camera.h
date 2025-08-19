#pragma once
#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/Math/Vector3.h"
#include "../Bamboo/Math/Matrix4.h"
namespace Bamboo
{
    class Camera
    {
    public:
        enum class ProjectionType
        {
            Orthographic,
            Perspective
        };

    public:
        Camera();
        ~Camera();
        /// @brief 设置 正交投影
        /// @param size 大小
        /// @param nearClip  近裁剪面
        /// @param farClip 远裁剪面
        void SetOrthographic(float size, float nearClip, float farClip);


        /// @brief 获取视图矩阵
        Matrix4 GetViewMatrix() const;
        /// @brief 获取投影矩阵
        Matrix4 GetProjectionMatrix() const;

    private:
        bool m_Orthographic;
        Vector3 m_Position;
        Vector3 m_Rotation;

        float m_OrthographicSize =10.0f;
        float m_OrhtographicNear = -10f;
        float m_OrhtographicFar = 10f;
    };

}