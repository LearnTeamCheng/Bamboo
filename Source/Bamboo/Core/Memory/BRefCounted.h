#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <utility>
#include <type_traits>

#include "BControlBlock.h"

namespace Bamboo
{
    class BRefCounted
    {
    public:
        BRefCounted() : m_ControlBlock(new BControlBlock())
        {
            m_ControlBlock->object = this;
        }

        void IncRef() const
        {
            m_ControlBlock->strongCount.fetch_add(1, std::memory_order_relaxed);
        }

        void DecRef() const
        {
            BControlBlock *controlBlock = m_ControlBlock;
            if (controlBlock->strongCount.fetch_sub(1, std::memory_order_relaxed) == 1)
            {
                controlBlock->object = nullptr;
                delete this;
                ReleaseWeakRef(controlBlock);
            }
        }

        uint32_t GetRefCount() const
        {
            return m_ControlBlock->strongCount.load(std::memory_order_relaxed);
        }

        BControlBlock *GetControlBlock() const noexcept
        {
            return m_ControlBlock;
        }

    protected:
        static void ReleaseWeakRef(BControlBlock *controlBlock);
        virtual ~BRefCounted() = default;

        BRefCounted(const BRefCounted &) = delete;
        BRefCounted &operator=(const BRefCounted &) = delete;

    private:
        mutable BControlBlock *m_ControlBlock;
    };

}