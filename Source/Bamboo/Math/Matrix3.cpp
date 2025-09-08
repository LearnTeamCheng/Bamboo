// 3x3矩阵
#include "Vector3.h"
#include "Matrix3.h"

namespace Bamboo
{
    Matrix3 Matrix3::operator*(const Matrix3 &other) const
    {
        Matrix3 result;

        for (size_t i = 0; i < 3; i++)
        {
            /* code */
            for (size_t j = 0; j < 3; j++)
            {
                result.m_data[i * 3 + j] = m_data[i * 3 + 0] * other.m_data[0 +j] + m_data[i * 3 + 1] * other.m_data[3+j] + m_data[i * 3 + 2] * other.m_data[6+j];
            }
        }
        return result;
    }

    Vector3 Matrix3::operator*(const Vector3 &other) const
    {
        Vector3 result;

        result.x = m_data[0] * other.x + m_data[3] * other.y + m_data[6] * other.z;
        result.y = m_data[1] * other.x + m_data[4] * other.y + m_data[7] * other.z;
        result.z = m_data[2] * other.x + m_data[5] * other.y + m_data[8] * other.z;
        return result;
    }

    Matrix3 Matrix3::operator+(const Matrix3 &other) const
    {
        Matrix3 result;
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                result.m_data[i*3+j] = m_data[i*3+j] + other.m_data[i*3+j];
            }
        }
        return result;
    }

    Matrix3 Matrix3::operator-(const Matrix3 &other) const
    {
        Matrix3 result;
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                result.m_data[i*3+j] = m_data[i*3+j] - other.m_data[i*3+j];
            }
        }
        return result;
    }

    bool Matrix3::operator==(const Matrix3 &other) const
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                if (m_data[i*3+j] != other.m_data[i*3+j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool Matrix3::operator!=(const Matrix3 &other) const
    {
        return !(*this == other);
    }


    // 转置矩阵
    Matrix3  Matrix3::Transpose() const
    {
        Matrix3 result;
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                result.m_data[i*3+j] = m_data[j*3+i];
            }
        }
        return result;
    }
    
    // 单位矩阵
    Matrix3 Matrix3::Identity()
    {
        static Matrix3 result;
        return result;
    }

    /**
     * [1,0,0]
     * [0,1,0]
     * [0,0,1]
     */
    // 缩放矩阵
    Matrix3 Matrix3::Scale(float x, float y, float z)
    {
        Matrix3 result;
        result.m_data[0] = x;
        result.m_data[4] = y;
        result.m_data[8] = z;
        return result;
    }

    // X轴旋转矩阵
    Matrix3 Matrix3::RotateX(float angle)
    {
        Matrix3 result;
        float cos = cosf(angle);
        float sin = sinf(angle);
        // result.m_data[1][1] = cos;
        // result.m_data[1][2] = -sin;
        // result.m_data[2][1] = sin;
        // result.m_data[2][2] = cos;
        result.m_data[4] = cos;
        result.m_data[8] = -sin;
        result.m_data[7] = sin;
        result.m_data[8] = cos;
        return result;
    }

    // Y轴旋转矩阵
    Matrix3 Matrix3::RotateY(float angle)
    {
        Matrix3 result;
        float cos = cosf(angle);
        float sin = sinf(angle);
        // result.m_data[0][0] = cos;
        // result.m_data[0][2] = sin;
        // result.m_data[2][0] = -sin;
        // result.m_data[2][2] = cos;

        result.m_data[0] = cos;
        result.m_data[6] = sin;
        result.m_data[2] = -sin;
        result.m_data[8] = cos;
        return result;
    }

    // Z轴旋转矩阵
    Matrix3 Matrix3::RotateZ(float angle)
    {
        Matrix3 result;
        float cos = cosf(angle);
        float sin = sinf(angle);
        // result.m_data[0][0] = cos;
        // result.m_data[0][1] = -sin;
        // result.m_data[1][0] = sin;
        // result.m_data[1][1] = cos;

        result.m_data[0] = cos;
        result.m_data[3] = -sin;
        result.m_data[1] = sin;
        result.m_data[4] = cos;
        return result;
    }

    // 平移矩阵
    Matrix3 Matrix3::Translate(float x, float y, float z)
    {
        Matrix3 result;
        // result.m_data[0][2] = x;
        // result.m_data[1][2] = y;
        // result.m_data[2][2] = z;

        result.m_data[2] = x;
        result.m_data[5] = y;
        result.m_data[8] = z;

        return result;
    }


};