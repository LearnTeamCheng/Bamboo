#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <utility>
#include <type_traits>

namespace Bamboo
{
    class BRefCounted
    {
    public:
        void IncRef() const
        {
            m_RefCount.fetch_add(1,std::memory_order_relaxed);
        }

        void DecRef() const
        {
            if (m_RefCount.fetch_sub(1,std::memory_order_acq_rel) == 1)
            {
                delete this;
            }
        }

        uint32_t GetRefCount() const
        {
            return m_RefCount.load(std::memory_order_relaxed);
        }

    protected:
        BRefCounted() = default;
        virtual ~BRefCounted() = default;

        BRefCounted(const BRefCounted&) = delete;
        BRefCounted& operator=(const BRefCounted&) = delete;

    private:
        mutable std::atomic<uint32_t> m_RefCount{0};
    };


    template<typename T>
    class BRef
    {
    public:
        BRef() noexcept = default;

        BRef(std::nullptr_t) noexcept
        {
        }

        explicit BRef(T* ptr) noexcept
            : m_Ptr(ptr)
        {
            AddRef();
        }

        BRef(const BRef& other) noexcept
            : m_Ptr(other.m_Ptr)
        {
            AddRef();
        }

        template<typename U>
        BRef(const BRef<U>& other)
        requires  std::is_convertible_v<U*, T*>
           : m_Ptr(other.Get())
        {
            AddRef();
        }

        BRef(BRef&& other) noexcept
            : m_Ptr(other.m_Ptr)
        {
            other.m_Ptr = nullptr;
        }

        ~BRef()
        {
            Release();
        }

        BRef& operator=(const BRef& other) noexcept
        {
            if (this != &other)
            {
                BRef temp(other);
                Swap(temp);
            }

            return *this;
        }

        BRef& operator=(BRef&& other) noexcept
        {
            if (this != &other)
            {
                Release();

                m_Ptr = other.m_Ptr;
                other.m_Ptr = nullptr;
            }

            return *this;
        }

        BRef& operator=(std::nullptr_t) noexcept
        {
            Release();
            return *this;
        }

        T* operator->() const noexcept
        {
            return m_Ptr;
        }

        T& operator*() const
        {
            return *m_Ptr;
        }

        T* Get() const noexcept
        {
            return m_Ptr;
        }

        explicit operator bool() const noexcept
        {
            return m_Ptr != nullptr;
        }

        void Reset() noexcept
        {
            Release();
        }

        void Swap(BRef& other) noexcept
        {
            std::swap(m_Ptr, other.m_Ptr);
        }

    private:
        void AddRef() noexcept
        {
            if (m_Ptr)
            {
                m_Ptr->IncRef();
            }
        }

        void Release() noexcept
        {
            if (m_Ptr)
            {
                m_Ptr->DecRef();
                m_Ptr = nullptr;
            }
        }

    private:
        T* m_Ptr = nullptr;
    };


}