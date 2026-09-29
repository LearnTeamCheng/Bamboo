#pragma once
#include <utility>
#include "BControlBlock.h"
#include "BRefPtr.h"
namespace Bamboo
{

    template <RefCounted T>
    class BWeakPtr
    {
    public:
        BWeakPtr() = default;
        BWeakPtr(std::nullptr_t) noexcept
        {
        }

        explicit BWeakPtr(BRefPtr<T> &refPtr) noexcept
        {
            Assign(refPtr);
        }

        BWeakPtr(const BWeakPtr &weakPtr) noexcept
            : m_ControlBlock(weakPtr.m_ControlBlock)
        {
            AddWeakRef();
        }

        BWeakPtr(BWeakPtr &&weakPtr) noexcept
            : m_ControlBlock(weakPtr.m_ControlBlock)
        {
            weakPtr.m_ControlBlock = nullptr;
        }

        ~BWeakPtr() noexcept
        {
            ReleaseWeakRef();
        }

        BWeakPtr &operator=(const BWeakPtr &other) noexcept
        {
            if (this != &other)
            {
                BWeakPtr temp(other);
                Swap(temp);
            }
            return *this;
        }

        BWeakPtr &operator=(const BWeakPtr &&other) noexcept
        {
            if (this != &other)
            {
                ReleaseWeakRef();
                m_ControlBlock = other.m_ControlBlock;
                other.m_ControlBlock = nullptr;
            }
            return *this;
        }

        BWeakPtr &operator=(const BRefPtr<T> &other) noexcept
        {
            ReleaseWeakRef();
            Assign(other);
            return *this;
        }

        BRefPtr<T> Lock() const noexcept
        {
            if (!m_ControlBlock)
            {
                return nullptr;
            }

            auto strongCount = m_ControlBlock->strongCount.load(std::memory_order_acquire);
            while (strongCount != 0)
            {
                if (m_ControlBlock->strongCount.compare_exchange_weak(strongCount, strongCount + 1, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    return BRefPtr<T>(static_cast<T *>(m_ControlBlock->object));
                }
            }
            return nullptr;
        }

        bool Expired() const noexcept
        {
            return !m_ControlBlock ||
                   m_ControlBlock->strongCount.load(std::memory_order_acquire) == 0;
        }

        bool IsValid() const noexcept
        {
            return !Expired();
        }

        void Reset() noexcept
        {
            ReleaseWeakRef();
        }

        void Swap(BWeakPtr &other)
        {
            std::swap(m_ControlBlock, other.m_ControlBlock);
        }

    private:
        void Assign(const BRefPtr<T> &ref)
        {
            if (!ref)
            {
                return;
            }

            m_ControlBlock = ref.Get()->GetControlBlock();
            AddWeakRef();
        }

        void AddWeakRef() noexcept
        {
            if (m_ControlBlock)
            {
                m_ControlBlock->weakCount.fetch_add(1, std::memory_order_relaxed);
            }
        }

        void ReleaseWeakRef() noexcept
        {
            if (!m_ControlBlock)
            {
                return;
            }

            if (m_ControlBlock->weakCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete m_ControlBlock;
            }
            m_ControlBlock = nullptr;
        }

        BControlBlock *m_ControlBlock{nullptr};
    };
};