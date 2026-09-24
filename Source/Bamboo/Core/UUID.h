#pragma once
/// @file UUID.h

#include <cstdint>
#include <iostream>
namespace Bamboo
{
    class UUID
    {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID &other) = default;
        operator uint64_t() const { return m_UUID; }
        static UUID Generate();
        bool IsValid() const
        {
            return m_UUID != 0;
        }

    private:
        uint64_t m_UUID{0};
    };
};

namespace std
{
    template <typename T>
    struct hash;

    template <>
    struct hash<Bamboo::UUID>
    {
        std::size_t operator()(const Bamboo::UUID &uuid) const
        {
            return (uint64_t)uuid;
        }
    };

}
