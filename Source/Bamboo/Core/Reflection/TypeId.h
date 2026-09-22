#pragma once
#include <iostream>
#include <typeindex>
namespace Bamboo
{

    using TypeId = std::type_index;

    template <typename T>
    TypeId GetTypeId()
    {
        return std::type_index(typeid(T));
    }
};