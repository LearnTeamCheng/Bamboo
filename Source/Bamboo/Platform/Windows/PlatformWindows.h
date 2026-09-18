// Bamboo/Platform/Windows/PlatformWindows.h
//
// Windows 平台头文件的统一入口。
// 这里在包含 <windows.h> 之前先把两个宏定好：
//   NOMINMAX          —— 阻止 windows.h 定义 min/max 宏，否则会污染 std::min/std::max
//   WIN32_LEAN_AND_MEAN —— 排除一批用不到的子系统头，明显加快编译
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>