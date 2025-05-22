
// Bamboo/Platform/Windows/PlatformWindows.h
#pragma once

#ifndef NOMINMAX
#define NOMINMAX // 避免 windows.h 定义 min/max 宏污染
#endif

#define WIN32_LEAN_AND_MEAN // 减少 windows.h 头文件大小，加快编译速度
#include <windows.h>