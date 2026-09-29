#include "BRefCounted.h"

namespace Bamboo
{
    void BRefCounted::ReleaseWeakRef(BControlBlock *controlBlock) 
    {
        if (controlBlock->weakCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            delete controlBlock; // 弱引用计数为0时，删除控制块
        }
    }
};
