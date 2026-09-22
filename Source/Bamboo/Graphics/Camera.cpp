#include "Camera.h"

namespace Bamboo
{
    void Camera::SetOrthographic(float size, float nearClip, float farClip)
    {
        m_OrthographicSize = size;
        m_OrthographicNear = nearClip;
        m_OrthographicFar = farClip;
        m_ProjectionType = ProjectionType::Orthographic;

        // 注意（已知缺陷，见 P1-6）：
        // 这里并不会立刻重算投影矩阵，而且重算时 m_OrthographicSize 会被
        // ReCalculateProjectionMatrix 里的 m_ViewportHeight * 0.5f 覆盖，
        // 所以传入的 size 目前是无效的 —— 相机无法缩放。
        // 修法：让投影矩阵惰性重算，并把 size 作为唯一缩放来源。
    }

    void Camera::SetViewportSize(uint32_t width, uint32_t height)
    {
        // TODO(相机): 这里的守卫无效 —— 判断了却什么都不做，且条件应为 ||。
        // 目前高度为 0 时，下一行的除法会产生 inf（见 P1-6）。
        if (width == 0 && height == 0)
        {
            // 无效的窗口
        }

        m_ViewportWidth = width;
        m_ViewportHeight = height;

        m_AspectRatio = (float)width / (float)height;
        ReCalculateProjectionMatrix();
    }

    void Camera::ReCalculateProjectionMatrix()
    {
        if (m_ProjectionType == ProjectionType::Orthographic)
        {
            // 视野高度由视口高度推导（像素相机：1 世界单位 = 1 像素），
            // 左右则按宽高比扩展。注意这里会覆盖 SetOrthographic 传入的 size。
            m_OrthographicSize = m_ViewportHeight * 0.5f;
            float left = -m_OrthographicSize * m_AspectRatio;
            float right = m_OrthographicSize * m_AspectRatio;
            float bottom = -m_OrthographicSize;
            float top = m_OrthographicSize;

            m_CameraData.left = left;
            m_CameraData.right = right;
            m_CameraData.bottom = bottom;
            m_CameraData.top = top;
            m_ProjectionMatrix = Matrix4::Orthographic(left, right, bottom, top, m_OrthographicNear, m_OrthographicFar);
        }
        else
        {
            // TODO(相机): ProjectionType::Perspective 没有任何实现，
            // 走到这里投影矩阵会保持单位阵（见 P1-6）。
        }
    }

    Vector3 Camera::ScreenToWorldPosition(const Vector3 &screenPosition) const
    {
        Vector3 worldPosition;
        worldPosition.x = m_CameraData.left + (screenPosition.x / m_ViewportWidth) * (m_CameraData.right - m_CameraData.left);
        worldPosition.y = m_CameraData.bottom + (screenPosition.y / m_ViewportHeight) * (m_CameraData.top - m_CameraData.bottom);
        worldPosition.z = 0.0f;
        return worldPosition;
    }

    Vector3 Camera::PixelSizeToWorldSize(const Vector2 &pixelSize) const
    {
        Vector3 worldSize;
        worldSize.x = (pixelSize.x / m_ViewportWidth) * (m_CameraData.right - m_CameraData.left);
        worldSize.y = (pixelSize.y / m_ViewportHeight) * (m_CameraData.top - m_CameraData.bottom);
        worldSize.z = 1.0f;
        return worldSize;
    }
    
    Matrix4 Camera::GetViewProjection() const
    {
        // 视图矩阵由外部（CameraSystem）通过 SetView 灌入，相机本身不持有位置。
        return m_ProjectionMatrix * m_ViewMatrix;
    }
}