/*
    2D Vector
*/
#pragma once

#include "Math.h"
class Vector3;
namespace Bamboo
{

    class Vector2
    {
    public:
        union
        {
            struct
            {
                float x, y;
            };
        };

    public:
        Vector2(float px, float py)
        {
            x = px;
            y = py;
        }

        Vector2() noexcept : x(0.0f), y(0.0f) {}
        Vector2(const Vector3 &v);

        Vector2(const Vector2 &other) noexcept = default;

        /// 负数
        Vector2 operator-() const { return Vector2(-x, -y); }

        Vector2 operator+(const Vector2 &other) const { return Vector2(x + other.x, y + other.y); }
        Vector2 operator-(const Vector2 &other) const { return Vector2(x - other.x, y - other.y); }
        Vector2 operator*(const float &other) const { return Vector2(x * other, y * other); }
        Vector2 operator/(const float &other) const { return Vector2(x / other, y / other); }

        friend static Vector2 operator*(const float &scalar, const Vector2 &vector)
        {
            return Vector2(vector.x * scalar, vector.y * scalar);
        }
        friend static Vector2 operator/(const float &scalar, const Vector2 &vector)
        {
            return Vector2(vector.x / scalar, vector.y / scalar);
        }

        //+=
        Vector2 &operator+=(const Vector2 &other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vector2 &operator-=(const Vector2 &other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vector2 &operator*=(const float &other)
        {
            x *= other;
            y *= other;
            return *this;
        }

        Vector2 &operator/=(const float &other)
        {
            x /= other;
            y /= other;
            return *this;
        }

        //==
        bool operator==(const Vector2 &other) const
        {
            return x == other.x && y == other.y;
        }
        bool operator!=(const Vector2 &other) const
        {
            return x != other.x || y != other.y;
        }

        bool IsZero() const
        {
            return Math::IsZero(x) && Math::IsZero(y);
        }

        // 判断是否在范围内
        bool IsEqual(const Vector2 &other, float epsilon = 0.0001f) const
        {
            return Math::IsEqual(x, other.x, epsilon) && Math::IsEqual(y, other.y, epsilon);
        }

        /// 数学函数

        // 向量取绝对值
        Vector2 Abs() const { return Vector2(Math::Abs(x), Math::Abs(y)); }

        // 向量向下取整
        Vector2 Floor() const { return Vector2(Math::Floor(x), Math::Floor(y)); }

        // 向量向上取整
        Vector2 Ceil() const { return Vector2(Math::Ceil(x), Math::Ceil(y)); }

        // 向量取整
        Vector2 Round() const { return Vector2(Math::Round(x), Math::Round(y)); }

        // 向量长度
        float Length() const { return Math::Sqrt(x * x + y * y); }

        // 向量长度
        float Magnitude() const { return Length(); }

        // 长度平方
        float LengthSquared() const { return x * x + y * y; }

        // 归一化
        void Normalize()
        {
            float length = Length();
            if (Math::IsZero(length))
            {
                return;
            }
            x /= length;
            y /= length;
        }

        Vector2 Normalized() const
        {
            Vector2 result = *this;
            result.Normalize();
            return result;
        }

        // 向量点乘
        float Dot(const Vector2 &other) const
        {
            return x * other.x + y * other.y;
        }
        // 向量叉乘
        Vector2 Cross(const Vector2 &other) const
        {
            return Vector2(y * other.x - x * other.y, x * other.y - y * other.x);
        }

        // 向量反射
        Vector2 Reflect(const Vector2 &normal) const
        {
            return *this - 2.0f * Dot(normal) * normal;
        }

        Vector2 Lerp(const Vector2 &other, float t) const
        {
            return *this * (1.0f - t) + other * t;
        }

        Vector2 Max(const Vector2 &other) const
        {
            return Vector2(Math::Max(x, other.x), Math::Max(y, other.y));
        }

        Vector2 Min(const Vector2 &other) const
        {
            return Vector2(Math::Min(x, other.x), Math::Min(y, other.y));
        }

        Vector2 Clamp(const Vector2 &min, const Vector2 &max) const
        {
            return Vector2(Math::Clamp(x, min.x, max.x), Math::Clamp(y, min.y, max.y));
        }

        // 常量
        static const Vector2 Zero;
        static const Vector2 One;
        static const Vector2 Up;
        static const Vector2 Down;
        static const Vector2 Left;
        static const Vector2 Right;
    };
};