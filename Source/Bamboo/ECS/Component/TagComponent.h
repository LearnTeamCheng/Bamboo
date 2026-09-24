
#pragma once
#include <string>
namespace Bamboo
{
    struct TagComponent
    {
        std::string tag;
        TagComponent() = default;
        TagComponent(const TagComponent &) = default;
        TagComponent(const std::string &tag) : tag(tag) {}
    };

}