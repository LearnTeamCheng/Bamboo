#include "Matrix4.h"

namespace Bamboo
{

    Matrix4 Matrix4::operator*(const Matrix4 &other) const
    {
        Matrix4 result;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result.m_data[i * 4 + j] = 0.0f;
                for (int k = 0; k < 4; k++)
                {
                    result.m_data[i * 4 + j] += m_data[i * 4 + k] * other.m_data[k * 4 + j];
                }
            }
        }
        return result;
    }

    /**创建一个平移的矩阵
     *  [1,0,0,x]
     *  [0,1,0,y]
     *  [0,0,1,z]
     *  [0,0,0,1]
     */
    Vector3 Matrix4::operator*(const Vector3 &other) const
    {
        Vector3 result;

        result.x = m_data[0] * other.x + m_data[1] * other.y + m_data[2] * other.z + m_data[3];
        result.y = m_data[4] * other.x + m_data[5] * other.y + m_data[6] * other.z + m_data[7];
        result.z = m_data[8] * other.x + m_data[9] * other.y + m_data[10] * other.z + m_data[11];

        return result;
    }

    Matrix4 Matrix4::operator+(const Matrix4 &other) const
    {
        Matrix4 result;
        for (int i = 0; i < 16; i++)
        {
            result.m_data[i] = m_data[i] + other.m_data[i];
        }
        return result;
    }

    Matrix4 Matrix4::operator-(const Matrix4 &other) const
    {
        Matrix4 result;
        for (int i = 0; i < 16; i++)
        {
            result.m_data[i] = m_data[i] - other.m_data[i];
        }
        return result;
    }

    Matrix4 Matrix4::operator/(float scalar) const
    {
        if (scalar == 0.0f)
        {
            // TODO: throw exception
            return *this;
        }

        Matrix4 result;
        for (int i = 0; i < 16; i++)
        {
            result.m_data[i] = m_data[i] / scalar;
        }
        return result;
    }

    Matrix4 Matrix4::Transpose() const
    {
        Matrix4 result;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result.m_data[i * 4 + j] = m_data[j * 4 + i];
            }
        }
        return result;
    }

    float Matrix4::Determinant() const
    {
        float a0 = m_data[0] * m_data[5] - m_data[1] * m_data[4];
        float a1 = m_data[0] * m_data[6] - m_data[2] * m_data[4];
        float a2 = m_data[0] * m_data[7] - m_data[3] * m_data[4];
        float a3 = m_data[1] * m_data[6] - m_data[2] * m_data[5];
        float a4 = m_data[1] * m_data[7] - m_data[3] * m_data[5];
        float a5 = m_data[2] * m_data[7] - m_data[3] * m_data[6];
        float b0 = m_data[8] * m_data[13] - m_data[9] * m_data[12];
        float b1 = m_data[8] * m_data[14] - m_data[10] * m_data[12];
        float b2 = m_data[8] * m_data[15] - m_data[11] * m_data[12];
        float b3 = m_data[9] * m_data[14] - m_data[10] * m_data[13];
        float b4 = m_data[9] * m_data[15] - m_data[11] * m_data[13];
        float b5 = m_data[10] * m_data[15] - m_data[11] * m_data[14];

        // Calculate the determinant.
        return (a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0);
    }

    Matrix4 Matrix4::Inverse() const
    {
        float a0 = m_data[0] * m_data[5] - m_data[1] * m_data[4];
        float a1 = m_data[0] * m_data[6] - m_data[2] * m_data[4];
        float a2 = m_data[0] * m_data[7] - m_data[3] * m_data[4];
        float a3 = m_data[1] * m_data[6] - m_data[2] * m_data[5];
        float a4 = m_data[1] * m_data[7] - m_data[3] * m_data[5];
        float a5 = m_data[2] * m_data[7] - m_data[3] * m_data[6];
        float b0 = m_data[8] * m_data[13] - m_data[9] * m_data[12];
        float b1 = m_data[8] * m_data[14] - m_data[10] * m_data[12];
        float b2 = m_data[8] * m_data[15] - m_data[11] * m_data[12];
        float b3 = m_data[9] * m_data[14] - m_data[10] * m_data[13];
        float b4 = m_data[9] * m_data[15] - m_data[11] * m_data[13];
        float b5 = m_data[10] * m_data[15] - m_data[11] * m_data[14];
        float det = (a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0);

        if (std::abs(det) < B_EPSILON)
        {
            return *this;
        }
        det = 1.0F / det;

        Matrix4 inverse;
        inverse.m_data[0] = (m_data[5] * b5 - m_data[6] * b4 + m_data[7] * b3) * det;
        inverse.m_data[1] = (-m_data[1] * b5 + m_data[2] * b4 - m_data[3] * b3) * det;
        inverse.m_data[2] = (m_data[13] * a5 - m_data[14] * a4 + m_data[15] * a3) * det;
        inverse.m_data[3] = (-m_data[9] * a5 + m_data[10] * a4 - m_data[11] * a3) * det;

        inverse.m_data[4] = (-m_data[4] * b5 + m_data[6] * b2 - m_data[7] * b1) * det;
        inverse.m_data[5] = (m_data[0] * b5 - m_data[2] * b2 + m_data[3] * b1) * det;
        inverse.m_data[6] = (-m_data[12] * a5 + m_data[14] * a2 - m_data[15] * a1) * det;
        inverse.m_data[7] = (m_data[8] * a5 - m_data[10] * a2 + m_data[11] * a1) * det;

        inverse.m_data[8] = (m_data[4] * b4 - m_data[5] * b2 + m_data[7] * b0) * det;
        inverse.m_data[9] = (-m_data[0] * b4 + m_data[1] * b2 - m_data[3] * b0) * det;
        inverse.m_data[10] = (m_data[12] * a4 - m_data[13] * a2 + m_data[15] * a0) * det;
        inverse.m_data[11] = (-m_data[8] * a4 + m_data[9] * a2 - m_data[11] * a0) * det;

        inverse.m_data[12] = (-m_data[4] * b3 + m_data[5] * b1 - m_data[6] * b0) * det;
        inverse.m_data[13] = (m_data[0] * b3 - m_data[1] * b1 + m_data[2] * b0) * det;
        inverse.m_data[14] = (-m_data[12] * a3 + m_data[13] * a1 - m_data[14] * a0) * det;
        inverse.m_data[15] = (m_data[8] * a3 - m_data[9] * a1 + m_data[10] * a0) * det;
        return inverse;
    }

    /*{
    2.0/(right - left),0,0,    -(right + left) / (right - left)
    0,2.0/(top - bottom),0,  -(top + bottom) / (top - bottom)
    0,0,-2.0/(zFar - zNear),-(zFar + zNear) / (zFar - zNear)
    0,0,0,1
    }**/
    Matrix4 Matrix4::Orthographic(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        Matrix4 result;
        result.m_data[0] = 2.0f / (right - left);
        result.m_data[5] = 2.0f / (top - bottom);
        result.m_data[10] = -2.0f / (zFar - zNear);
        result.m_data[3] = -(right + left) / (right - left);
        result.m_data[7] = -(top + bottom) / (top - bottom);
        result.m_data[11] = -(zFar + zNear) / (zFar - zNear);
        result.m_data[15] = 1.0f;

        // result.m_data[0] = 2.0f / (right - left);
        // result.m_data[5] = 2.0f / (top - bottom);
        // result.m_data[10] = -2.0f / (zFar - zNear);

        // result.m_data[12] = -(right + left) / (right - left);
        // result.m_data[13] = -(top + bottom) / (top - bottom);
        // result.m_data[14] = -(zFar + zNear) / (zFar - zNear);
        
        // result.m_data[15] = 1.0f;

        // Matrix4 result;

        // result.m_data[0] = 2.0f / (right - left);
        // result.m_data[5] = 2.0f / (top - bottom);
        // result.m_data[10] = -2.0f / (zFar - zNear);

        // result.m_data[12] = -(right + left) / (right - left);
        // result.m_data[13] = -(top + bottom) / (top - bottom);
        // result.m_data[14] = -(zFar + zNear) / (zFar - zNear);

        // result.m_data[15] = 1.0f;

        return result;
    }

    /**创建一个平移的矩阵
     *  [1,0,0,x]
     *  [0,1,0,y]
     *  [0,0,1,z]
     *  [0,0,0,1]
     */
    Matrix4 Matrix4::Translate(const Vector3 &translation)
    {
        Matrix4 result;
        result.m_data[3] = translation.x;
        result.m_data[7] = translation.y;
        result.m_data[11] = translation.z;
        return result;
    }
    /**
    {
        x,0,0,0,
        0,y,0,0,
        0,0,z,0,
        0,0,0,1
    }
    **/
    Matrix4 Matrix4::Scale(const Vector3 &scale)
    {
        Matrix4 result;
        result.m_data[0] = scale.x;
        result.m_data[5] = scale.y;
        result.m_data[10] = scale.z;
        return result;
    }

    Matrix4 Matrix4::LookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up)
    {

        Vector3 zAxis = (target - eye).Normalized();
        Vector3 xAxis = up.Cross(zAxis).Normalized();
        Vector3 yAxis = zAxis.Cross(xAxis);

        Matrix4 result;
        result.m_data[0] = xAxis.x;
        result.m_data[1] = yAxis.x;
        result.m_data[2] = zAxis.x;
        result.m_data[3] = -xAxis.Dot(eye);

        result.m_data[4] = xAxis.y;
        result.m_data[5] = yAxis.y;
        result.m_data[6] = zAxis.y;
        result.m_data[7] = -yAxis.Dot(eye);

        result.m_data[8] = xAxis.z;
        result.m_data[9] = yAxis.z;
        result.m_data[10] = zAxis.z;
        result.m_data[11] = -zAxis.Dot(eye);

        result.m_data[15] = 1.0f;

        return result;
    }

    // TODO: 旋转矩阵的实现
    Matrix4 Matrix4::RotateXYZ(const Vector3 &rotation)
    {
        Matrix4 result;
        float cosX = cos(rotation.x);
        float sinX = sin(rotation.x);
        float cosY = cos(rotation.y);
        float sinY = sin(rotation.y);
        float cosZ = cos(rotation.z);
        float sinZ = sin(rotation.z);

        result.m_data[0] = cosY * cosZ;
        result.m_data[1] = cosY * sinZ;
        result.m_data[2] = -sinY;
        result.m_data[3] = 0.0f;

        result.m_data[4] = sinX * sinY * cosZ - cosX * sinZ;
        result.m_data[5] = cosX * cosZ + sinX * sinY * sinZ;
        result.m_data[6] = sinX * cosY;
        result.m_data[7] = 0.0f;

        result.m_data[8] = cosX * sinY * cosZ + sinX * sinZ;
        result.m_data[9] = -sinX * cosZ + cosX * sinY * sinZ;
        result.m_data[10] = cosX * cosY;
        result.m_data[11] = 0.0f;

        result.m_data[15] = 1.0f;

        return result;
    }

    Matrix4 Matrix4::RotateZ(float angle){
        Matrix4 result;
        float cosA = cos(angle);
        float sinA = sin(angle);

        result.m_data[0] = cosA;
        result.m_data[1] = -sinA;
        result.m_data[2] = 0.0f;
        result.m_data[3] = 0.0f;

        result.m_data[4] = sinA;
        result.m_data[5] = cosA;
        result.m_data[6] = 0.0f;
        result.m_data[7] = 0.0f;

        result.m_data[8] = 0.0f;
        result.m_data[9] = 0.0f;
        result.m_data[10] = 1.0f;
        result.m_data[11] = 0.0f;

        result.m_data[15] = 1.0f;
        return result;
    }
}