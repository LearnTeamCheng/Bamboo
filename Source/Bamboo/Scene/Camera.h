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

        /// @brief 获取投影矩阵
        Matrix4 GetProjectionMatrix() const {return m_ProjectionMatrix;}
        /// @brief 获取视图矩阵
        Matirx4 GetViewMatrix() const;
        /// @brief 获取视图投影矩阵
        Matrix4 GetViewProjectionMatrix() const;

        void SetPosition(const Vector3& position) {m_Position = position;}
        const Vector3& GetPosition() const {return m_Position;}

        void SetRotation(const Vector3& rotation) {m_Rotation = rotation;}
        const Vector3& GetRotation() const {return m_Rotation;}

        /// @brief 窗口大小变
        void SetViewportSize(uint32_t width, uint32_t height);
    protected:
        /// @brief 计算投影矩阵
        void ReCalculateProjectionMatrix();

    private:
        bool m_Orthographic;
        Vector3 m_Position;
        Vector3 m_Rotation;

        float m_AspectRatio = 1.0f;

        float m_OrthographicSize =10.0f;
        float m_OrhtographicNear = -10.0f;
        float m_OrhtographicFar = 10.0f;

        Matrix4 m_ProjectionMatrix;
        ProjectionType m_ProjectionType;
    };

}