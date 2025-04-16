#pragma once
#include "Math.h"
namespace Bamboo
{

    class IntVector2
    {
    public:
        union
        {
            struct
            {
                int x, y;
            };
        };

    public:
        IntVector2() : x(0), y(0) {}
        IntVector2(int x, int y) : x(x), y(y) {}
        IntVector2(const IntVector2 &other) noexcept = default;

        IntVector2 operator-() const { return IntVector2(-x, -y); }

        IntVector2 operator+(const IntVector2 &other) const { return IntVector2(x + other.x, y + other.y); }
        IntVector2 operator-(const IntVector2 &other) const { return IntVector2(x - other.x, y - other.y); }
        IntVector2 operator*(const IntVector2 &other) const { return IntVector2(x * other.x, y * other.y); }
        IntVector2 operator/(const IntVector2 &other) const { return IntVector2(x / other.x, y / other.y); }

        friend static IntVector2 operator*(int scalar, const IntVector2 &vector) { return IntVector2(scalar * vector.x, scalar * vector.y); }
        friend static IntVector2 operator*(const IntVector2 &vector, int scalar) { return IntVector2(scalar * vector.x, scalar * vector.y); }
        friend static IntVector2 operator/(int scalar, const IntVector2 &vector) { return IntVector2(scalar / vector.x, scalar / vector.y); }
        friend static IntVector2 operator/(const IntVector2 &vector, int scalar) { return IntVector2(vector.x / scalar, vector.y / scalar); }

        IntVector2 &operator+=(const IntVector2 &other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        IntVector2 &operator-=(const IntVector2 &other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        IntVector2 &operator*=(const IntVector2 &other)
        {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        IntVector2 &operator/=(const IntVector2 &other)
        {
            x /= other.x;
            y /= other.y;
            return *this;
        }

        bool operator==(const IntVector2 &other) const { return x == other.x && y == other.y; }
        bool operator!=(const IntVector2 &other) const { return x != other.x || y != other.y; }

        bool IsZero() const { return x == 0 && y == 0; }
        bool IsOne() const { return x == 1 && y == 1; }

        int Dot(const IntVector2 &other) const { return x * other.x + y * other.y; }
        int Cross(const IntVector2 &other) const { return x * other.y - y * other.x; }

        int LengthSquared() const { return x * x + y * y; }
        int Length() const { return Math::Sqrt(LengthSquared()); }

        IntVector2 Normalize() const { return *this / Length(); }
        IntVector2 Abs() const { return IntVector2(Math::Abs(x), Math::Abs(y)); }
        // 向量的反转
        IntVector2 Perpendicular() const { return IntVector2(-y, x); }

        IntVector2 Clamp(const IntVector2 &min, const IntVector2 &max) const
        {
            return IntVector2(Math::Max(min.x, Math::Min(max.x, x)), Math::Max(min.y, Math::Min(max.y, y)));
        }

        IntVector2 Lerp(const IntVector2 &other, float t) const
        {
            return IntVector2(x + (other.x - x) * t, y + (other.y - y) * t);
        }

        // 向量反射
        IntVector2 Reflect(const IntVector2 &normal) const
        {
            return *this - 2 * Dot(normal) * normal;
        }

        // 常量定义
        static const IntVector2 Zero;
        static const IntVector2 One;
        static const IntVector2 Up;
        static const IntVector2 Down;
        static const IntVector2 Left;
        static const IntVector2 Right;
    };
};