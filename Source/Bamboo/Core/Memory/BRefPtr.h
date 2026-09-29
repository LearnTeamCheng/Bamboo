#pragma once

#include <utility>
#include "BRefCounted.h"
namespace Bamboo
{
    template<typename T>
    concept RefCounted =std::derived_from<T, BRefCounted>;

    template <typename T>
    class BRefPtr
    {
    public:
        BRefPtr() noexcept = default;

        BRefPtr(std::nullptr_t) noexcept
        {
        }

        explicit BRefPtr(T *ptr) noexcept
            : m_Ptr(ptr)
        {
        }

        BRefPtr(const BRefPtr &other) noexcept
            : m_Ptr(other.m_Ptr)
        {
            AddRef();
        }

        template <RefCounted U>
        requires std::is_convertible_v<U *, T *>
        BRefPtr(const BRefPtr<U> &other)
            : m_Ptr(other.Get())
        {
            AddRef();
        }

        BRefPtr(BRefPtr &&other) noexcept
            : m_Ptr(other.m_Ptr)
        {
            other.m_Ptr = nullptr;
        }

        ~BRefPtr()
        {
            Release();
        }

        BRefPtr &operator=(const BRefPtr &other) noexcept
        {
            if (this != &other)
            {
                BRefPtr temp(other);
                Swap(temp);
            }

            return *this;
        }

        BRefPtr &operator=(BRefPtr &&other) noexcept
        {
            if (this != &other)
            {
                Release();

                m_Ptr = other.m_Ptr;
                other.m_Ptr = nullptr;
            }

            return *this;
        }

        BRefPtr &operator=(std::nullptr_t) noexcept
        {
            Release();
            return *this;
        }

        T *operator->() const noexcept
        {
            return m_Ptr;
        }

        T &operator*() const
        {
            return *m_Ptr;
        }

        T *Get() const noexcept
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

        void Swap(BRefPtr& other) noexcept
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
        T *m_Ptr = nullptr;
    };



};