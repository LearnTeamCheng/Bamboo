#pragma once
#include <atomic>
namespace Bamboo
{
    struct BControlBlock
    {
        std::atomic<int> strongCount{1}; // 强引用计数
        std::atomic<int> weakCount{1};   // 弱引用计数
        class BRefCounted *object{nullptr};
    };
}