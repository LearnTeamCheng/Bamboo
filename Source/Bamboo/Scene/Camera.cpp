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

        m_AspectRatio = (float)width / (float)height;
        ReCalculateProjectionMatrix();
    }

    Matrix4 Camera::GetViewMatrix() const
    {
    }

    void Camera::ReCalculateProjectionMatrix()
    {
        if (m_ProjectionType == ProjectionType::Orthographic)
        {
            float left = -m_OrthographicSize * m_AspectRatio * 0.5f;
            float right = m_OrthographicSize * m_AspectRatio * 0.5f;
            float bottom = -m_OrthographicSize * 0.5f;
            float top = m_OrthographicSize * 0.5f;
            m_ProjectionMatrix = Matrix4::Orthographic(left, right, bottom, top, m_OrhtographicNear, m_OrhtographicFar);
        }
        else
        {
        }
    }
}