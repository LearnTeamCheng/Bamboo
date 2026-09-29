#pragma once
#include <utility>
#include "BRefPtr.h"
#include "BWeakPtr.h"
namespace Bamboo
{
    template <typename T, typename... Args>
    constexpr BRefPtr<T> CreateBRef(Args &&...args)
    {
        return BRefPtr<T>(new T(std::forward<Args>(args)...));
    }

}