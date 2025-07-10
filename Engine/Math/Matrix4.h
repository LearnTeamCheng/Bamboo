/**
 * @file Matrix4.h
 * @brief 4x4 矩阵类.
 *
 */
#pragma once
#include "../Math/Vector3.h"

namespace Bamboo
{
    class Matrix4
    {
    public:
        Matrix4() : m_data{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f} {}

        float &operator()(int row, int col) { return m_data[row * 4 + col]; }
        const float &operator()(int row, int col) const { return m_data[row * 4 + col]; }
        ///运算符重载
        Matrix4 operator*(const Matrix4 &other) const;
        Matrix4 operator+(const Matrix4 &other) const;
        Matrix4 operator-(const Matrix4 &other) const;
        Matrix4 operator/(float scalar) const;

        Matrix4 &operator*=(const Matrix4 &other);
        Matrix4 &operator+=(const Matrix4 &other);
        Matrix4 &operator-=(const Matrix4 &other);
        Matrix4 &operator/=(float scalar);

        ///矩阵转置  
        Matrix4 Transpose() const;
        ///矩阵逆
        Matrix4 Inverse() const;
        ///行列式
        float Determinant() const;
        
        static Matrix4 Identity();
        static Matrix4 Translate(const Vector3 &translation);
        static Matrix4 Scale(const Vector3 &scale);
        static Matrix4 RotateX(float angle);
        static Matrix4 RotateY(float angle);
        static Matrix4 RotateZ(float angle);



    private:
        float m_data[16];
    };
};