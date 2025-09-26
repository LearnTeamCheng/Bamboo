#pragma once
#include "../Bamboo/Core/Ref.h"
#include "../Bamboo/Math/Vector2.h"
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
        Camera() =default;
        ~Camera() = default;
        /// @brief 设置 正交投影
        /// @param size 大小
        /// @param nearClip  近裁剪面
        /// @param farClip 远裁剪面
        void SetOrthographic(float size, float nearClip, float farClip);

        /// @brief 获取投影矩阵
        Matrix4 GetProjection() const {return m_ProjectionMatrix;}
        /// @brief 获取视图矩阵
        //Matrix4 GetViewMatrix() const;
        /// @brief 获取视图投影矩阵
        Matrix4 GetViewProjectionMatrix() const;

        void SetPosition(const Vector3& position) {m_Position = position;}
        const Vector3& GetPosition() const {return m_Position;}

        void SetRotation(const Vector3& rotation) {m_Rotation = rotation;}
        const Vector3& GetRotation() const {return m_Rotation;}

        /// @brief 窗口大小变
        void SetViewportSize(uint32_t width, uint32_t height);

        float GetOrthographicSize()const { return m_OrthographicSize; }
        /// @brief 获取宽高比 
        float GetAspectRatio()const { return m_AspectRatio; }

        size_t GetViewportWidth()const { return m_ViewportWidth; }
        size_t GetViewportHeight() const {return m_ViewportHeight;}

        Vector3 ScreenToWorldPosition(const Vector3& screenPos) const;
        Vector3 PixelSizeToWorldSize(const Vector2& pixelSize) const;

        void SetPrimaryCamera(bool primary) { m_PrimaryCamera = primary; }
        bool IsPrimaryCamera() const { return m_PrimaryCamera; }

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

        size_t m_ViewportWidth = 0;
        size_t m_ViewportHeight = 0;

        Matrix4 m_ProjectionMatrix;
        ProjectionType m_ProjectionType;

        struct CameraData
        {
           float left;
            float right;
            float bottom;
            float top;
        };

        CameraData m_CameraData;
        /// @brief 是否是主摄像机
        bool m_PrimaryCamera = false;
    };

}