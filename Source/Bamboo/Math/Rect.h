/**
 * @file Rect.h
 * @brief 矩形类
 */
#pragma once

#include "Math.h"
#include <utility>
namespace Bamboo
{   
    class Vector2;
    class Rect {
        public:
            Rect()
                : x(0.0f), y(0.0f), width(0.0f), height(0.0f) {}
            Rect(float x, float y, float width, float height)
                : x(x), y(y), width(width), height(height) {}

            Rect( const Vector2 & pos, const Vector2 &size);
            Rect( const Rect & rect) = default;
            Rect& operator=( const Rect & rect) = default;

        
            
            // 包含另一个矩形
            bool Contains(const Rect& rect) const {
                return rect.x >= this->x && rect.x + rect.width <= this->x + this->width && rect.y >= this->y && rect.y + rect.height <= this->y + this->height;
            }
            // 包含某个点
            bool Contains(float x, float y) const {
                return x >= this->x && x < this->x + this->width && y >= this->y && y < this->y + this->height;
            }

            // 交集
            Rect Intersect(const Rect& rect) const {
                float x1 = Math::Max(this->x, rect.x);
                float y1 = Math::Max(this->y, rect.y);
                float x2 = Math::Min(this->x + this->width, rect.x + rect.width);
                float y2 = Math::Min(this->y + this->height, rect.y + rect.height);
                if (x2 < x1 || y2 < y1) {
                    return Rect();
                }
                return Rect(x1, y1, x2 - x1, y2 - y1);
            }

            // 相交
            bool IsIntersect(const Rect& rect) const {
                return this->x < rect.x + rect.width && this->x + this->width > rect.x && this->y < rect.y + rect.height && this->y + this->height > rect.y;
            }

            // 并集
            Rect UnionRect(const Rect& rect) const {
                int x1 = std::min(this->x, rect.x);
                int y1 = std::min(this->y, rect.y);
                int x2 = std::max(this->x + this->width, rect.x + rect.width);
                int y2 = std::max(this->y + this->height, rect.y + rect.height);
                return Rect(x1, y1, x2 - x1, y2 - y1);
            }


        public:
            float x;
            float y;
            float width;
            float height;
    };

    class Vector2Int ;
    
    class RectInt {
        public:
            RectInt()
                : left(0), top(0), right(0), bottom(0) {}
            RectInt(int left, int top, int right, int bottom)
                : left(left), top(top), right(right), bottom(bottom) {}

            RectInt( const Vector2Int & pos, const Vector2Int &size);
            RectInt( const RectInt & rect) = default;
            RectInt& operator=( const RectInt & rect) = default;
        public:
            int left, top, right, bottom;
    };
};