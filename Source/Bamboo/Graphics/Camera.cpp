#include "Camera.h"
namespace Bamboo
{
    void Camera::SetOrthographic(float size, float nearClip, float farClip)
    {
        m_OrthographicSize = size;
        m_OrhtographicNear = nearClip;
        m_OrhtographicFar = farClip;
        m_ProjectionType = ProjectionType::Orthographic;
    }

    void Camera::SetViewportSize(uint32_t width, uint32_t height)
    {

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
            float left = -m_OrthographicSize * m_AspectRatio * 0.5f;
            float right = m_OrthographicSize * m_AspectRatio * 0.5f;
            float bottom = -m_OrthographicSize * 0.5f;
            float top = m_OrthographicSize * 0.5f;

            m_CameraData.left = left;
            m_CameraData.right = right;
            m_CameraData.bottom = bottom;
            m_CameraData.top = top;
            m_ProjectionMatrix = Matrix4::Orthographic(left, right, bottom, top, m_OrhtographicNear, m_OrhtographicFar);
        }
        else
        {
        }
    }

    Vector3 Camera::ScreenToWorldPosition(const Vector3& screenPosition) const{
        Vector3 worldPosition;
        worldPosition.x = m_CameraData.left + (screenPosition.x / m_ViewportWidth) * (m_CameraData.right - m_CameraData.left);
        worldPosition.y = m_CameraData.bottom + (screenPosition.y / m_ViewportHeight) * (m_CameraData.top - m_CameraData.bottom);
        worldPosition.z = 0.0f;
        return worldPosition;
    }

    Vector3 Camera::PixelSizeToWorldSize(const Vector2& pixelSize) const 
    {
        Vector3 worldSize;
        worldSize.x = (pixelSize.x / m_ViewportWidth) * (m_CameraData.right - m_CameraData.left);
        worldSize.y = (pixelSize.y / m_ViewportHeight) * (m_CameraData.top - m_CameraData.bottom);
        worldSize.z = 1.0f;
        return worldSize;
    }
}