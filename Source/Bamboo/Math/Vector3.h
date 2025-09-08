#pragma once
#include "Math.h"

class Vector2;
class Vector4;


namespace Bamboo
{
    class Vector3
    {
    public:
        union
        {
            struct
            {
                float x, y, z;
            };
        };

    public:
        Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
        Vector3(const Vector3 &other) = default;
        //Vector3(const Vector4 &other);
        
        Vector3 &operator=(const Vector3 &other) = default;



        Vector3(const Vector2 &v);
        ~Vector3() = default;

        float Dot(const Vector3 &other) const
        {
            return x * other.x + y * other.y + z * other.z;
        }

        Vector3 Cross(const Vector3 &other) const
        {
            return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
        }



        float Length() const
        {
            return Math::Sqrt(x * x + y * y + z * z);
        }

        void Normalize()
        {
            float length = Length();
            if (Math::IsZero(length))
            {
                return;
            }
            x /= length;
            y /= length;
            z /= length;
        }

        Vector3 Normalize() const
        {
            Vector3 result = *this;
            result.Normalize();
            return result;
        }

        Vector3 operator+(const Vector3 &other) const
        {
            return Vector3(x + other.x, y + other.y, z + other.z);
        }

        Vector3 operator-(const Vector3 &other) const
        {
            return Vector3(x - other.x, y - other.y, z - other.z);
        }

        Vector3 operator*(float scalar) const
        {
            return Vector3(x * scalar, y * scalar, z * scalar);
        }

        Vector3 operator/(float scalar) const
        {
            return Vector3(x / scalar, y / scalar, z / scalar);
        }

        Vector3 &operator+=(const Vector3 &other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }
        Vector3 &operator-=(const Vector3 &other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }
        Vector3 &operator*=(float scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }
        Vector3 &operator/=(float scalar)
        {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        Vector3 operator-() const
        {
            return Vector3(-x, -y, -z);
        }

        bool operator==(const Vector3 &other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }

        bool operator!=(const Vector3 &other) const
        {
            return x != other.x || y != other.y || z != other.z;
        }

        bool IsZero() const
        {
            return Math::IsZero(x) && Math::IsZero(y) && Math::IsZero(z);
        }

        void Clamp(const Vector3 &min, const Vector3 &max)
        {
            x = Math::Clamp(x, min.x, max.x);
            y = Math::Clamp(y, min.y, max.y);
            z = Math::Clamp(z, min.z, max.z);
        }

        void Lerp(const Vector3 &other, float t)
        {
            x = Math::Lerp(x, other.x, t);
            y = Math::Lerp(y, other.y, t);
            z = Math::Lerp(z, other.z, t);
        }

        void Min(const Vector3 &other)
        {
            x = Math::Min(x, other.x);
            y = Math::Min(y, other.y);
            z = Math::Min(z, other.z);
        }

        void Max(const Vector3 &other)
        {
            x = Math::Max(x, other.x);
            y = Math::Max(y, other.y);
            z = Math::Max(z, other.z);
        }

        friend Vector3 operator*(float scalar, const Vector3 &vector)
        {
            return Vector3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
        }

        //反射 
        Vector3 Reflect(const Vector3 &normal) const
        {
            return *this - 2.0f * Dot(normal) * normal;
        }



    public:
        // 常量定义

        static const Vector3 Zero;
        static const Vector3 One;
        static const Vector3 Forward;
        static const Vector3 Backward;
        static const Vector3 Up;
        static const Vector3 Down;
        static const Vector3 Left;
        static const Vector3 Right;
    };
}