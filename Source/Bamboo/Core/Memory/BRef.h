#pragma once
#include <utility>
#include "BRefCounted.h"
namespace Bamboo {

    template<typename T,typename... Args>
    constexpr BRef<T> CreateBRef(Args&& ... args)
    {
        return BRef<T>(new T(std::forward<Args>(args)...)); 
    }

};