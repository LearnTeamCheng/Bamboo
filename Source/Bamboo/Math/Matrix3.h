/**
 *  @file   Matrix3.h
 *  @brief  3x3矩阵.
 *  @author 
*/

#pragma once

namespace Bamboo
{
    class Vector3;
    // 3x3 matrix
    class Matrix3
    {
    public:
        Matrix3() : m_data{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f} {}
        Matrix3(float a, float b, float c, float d, float e, float f, float g, float h, float i) : m_data{a, b, c, d, e, f, g, h, i} {}

        float& operator()(int row, int col) { return m_data[row + col * 3]; }
        const float& operator()(int row, int col) const { return m_data[row + col * 3]; }

        Matrix3 operator*(const Matrix3& other) const;
        Matrix3 operator+(const Matrix3& other) const;
        Matrix3 operator-(const Matrix3& other) const;
        Matrix3 operator/(float scalar) const;

        Matrix3& operator*=(float scalar);
        Matrix3& operator*=(const Matrix3& other);
        Matrix3& operator+=(const Matrix3& other);
        Matrix3& operator-=(const Matrix3& other);
        Matrix3 operator*(float scalar) const;

        Vector3 operator*(const Vector3& other) const;

        Matrix3& operator/=(float scalar);
        bool operator==(const Matrix3& other) const;
        bool operator!=(const Matrix3& other) const;

        float* Data() { return &m_data[0]; }
        const float* Data() const { return &m_data[0]; }

  

        //矩阵转置
        Matrix3 Transpose() const;

        static Matrix3 Identity();
        static Matrix3 Scale(float x, float y, float z);
        static Matrix3 Translate(float x, float y, float z);
        static Matrix3 RotateX(float angle);
        static Matrix3 RotateY(float angle);
        static Matrix3 RotateZ(float angle);
        static Matrix3 Orthographic(float left, float right, float bottom, float top, float near, float far);
        static Matrix3 Perspective(float fov, float aspect, float near, float far);

 
    private:
        float m_data[9];
    };
}