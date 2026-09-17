#pragma once
#include "Bamboo/Math/Color.h"

namespace Bamboo::Shape
{
    struct RectangleComponent
    {
        Color color;
        float width;
        float height;
        bool filled;
        RectangleComponent(Color color, float width, float height, bool filled = false) : color(color), width(width), height(height), filled(filled) {} // Constructor
        RectangleComponent() : color(Color::White), width(0), height(0), filled(false) {}                                                               // Default constructor
    };
};
