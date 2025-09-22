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
        float det = 0.0f;
        for (int i = 0; i < 4; i++)
        {
            det += m_data[i * 4 + 0] * (m_data[1 + (i + 1) % 4] * m_data[5 + (i + 2) % 4] - m_data[2 + (i + 2) % 4] * m_data[4 + (i + 1) % 4]);
        }
        return det;
    }

    Matrix4 Matrix4::Inverse() const
    {
        float det = Determinant();
        if (det == 0.0f)
        {
            // TODO: throw exception
            return *this;
        }

        Matrix4 result;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                float sign = ((i + j) % 2 == 0) ? 1.0f : -1.0f;
                float cofactor = (m_data[(j + 1) % 4 * 4 + i] * m_data[(j + 2) % 4 * 4 + (i + 1) % 4] - m_data[(j + 2) % 4 * 4 + i] * m_data[(j + 1) % 4 * 4 + (i + 1) % 4]) / det;
                result.m_data[i * 4 + j] = sign * cofactor;
            }
        }
        return result;
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

        //result.m_data[0] = 2.0f / (right - left);
        //result.m_data[5] = 2.0f / (top - bottom);
        //result.m_data[10] = -2.0f / (zFar - zNear);
        //result.m_data[12] = -(right + left) / (right - left);
        //result.m_data[13] = -(top + bottom) / (top - bottom);
        //result.m_data[14] = -(zFar + zNear) / (zFar - zNear);
        //result.m_data[15] = 1.0f;
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
     Matrix4 Matrix4::Scale(const Vector3& scale) 
     {
         Matrix4 result;
         result.m_data[0] = scale.x;
         result.m_data[5] = scale.x;
         result.m_data[10] = scale.x;
         return result;
     }
}