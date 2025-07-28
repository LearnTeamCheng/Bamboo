#pragma once

/// 定义一个宏，用于展开一个宏
#define BAMBOO_EXPAND_MACRO(x) x
/// 定义一个宏，用于展开一个宏，并将展开后的结果转换为字符串
#define BAMBOO_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

