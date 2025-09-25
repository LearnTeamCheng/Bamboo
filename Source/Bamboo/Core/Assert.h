#pragma once
#include "Base.h"
#include "Log.h"

namespace Bamboo {

#define BAMBOO_ASSESERT(check, ...)                                                   \
    {                                                                             \
        if (!check)                                                                \
        {                                                                         \
            BAMBOO_CORE_ERROR("Assertion Failed: {0} {1}", BAMBOO_STRINGIFY_MACRO(check) ,check); \
            BAMBOO_CORE_ERROR(__VA_ARGS__);                                        \
        }                                                                         \
    }

}