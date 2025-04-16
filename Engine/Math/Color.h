/**
 * @file Color.h
 * @author <NAME> (<EMAIL>)
 * @brief 颜色类
 */
#pragma once
#include "Math.h"
namespace Bamboo
{
    class Color
    {
    public:
        float r, g, b, a;

    public:
        Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
        Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

        Color(const Color &other) : r(other.r), g(other.g), b(other.b), a(other.a) {}

        Color &operator=(const Color &other) = default;

        bool operator==(const Color &other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        bool operator!=(const Color &other) const
        {
            return r != other.r || g != other.g || b != other.b || a != other.a;
        }

        // 重载运算符
        Color operator+(const Color &other) const
        {
            return Color(r + other.r, g + other.g, b + other.b, a + other.a);
        }

        Color operator-(const Color &other) const
        {
            return Color(r - other.r, g - other.g, b - other.b, a - other.a);
        }

        Color operator*(float scalar) const
        {
            return Color(r * scalar, g * scalar, b * scalar, a * scalar);
        }

        Color operator-() const
        {
            return Color(-r, -g, -b, -a);
        }

        Color &operator+=(const Color &other)
        {
            r += other.r;
            g += other.g;
            b += other.b;
            a += other.a;
            return *this;
        }

        Color Abs() const
        {
            return Color(Math::Abs(r), Math::Abs(g), Math::Abs(b), Math::Abs(a));
        }

        Color Lerp(const Color &other, float t) const
        {
            return Color(Math::Lerp(r, other.r, t), Math::Lerp(g, other.g, t), Math::Lerp(b, other.b, t), Math::Lerp(a, other.a, t));
        }
        

    public:
        // 常量定义

        // 黑色
        static const Color Black;
        // 白色
        static const Color White;
        // 红色
        static const Color Red;
        // 绿色
        static const Color Green;
        // 蓝色
        static const Color Blue;
        // 黄色
        static const Color Yellow;
        // 青色
        static const Color Cyan;
        // 品红色
        static const Color Magenta;
        // 透明色
        static const Color Transparent;
    };
}; // namespace Bamboo