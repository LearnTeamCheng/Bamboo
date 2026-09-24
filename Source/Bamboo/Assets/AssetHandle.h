#pragma once
#include "../Core/UUID.h"

namespace Bamboo
{

    class AssetHandle
    {
    public:
        AssetHandle() = default;
        explicit AssetHandle(UUID id)
            : m_id(id)
        {
        }

        UUID GetUUID() const
        {
            return m_id;
        }

        bool IsValid() const
        {
            return m_id.IsValid();
        }

        std::size_t operator()(const Bamboo::AssetHandle &handle) const noexcept
        {
            return static_cast<std::size_t>(handle.GetUUID());
        }

        // bool operator==(const AssetHandle &other) const = default;

        bool operator==(const AssetHandle &other) const
        {
            return GetUUID() == other.GetUUID();
        }

    private:
        UUID m_id;
    };
};

namespace std
{
    template <>
    struct hash<Bamboo::AssetHandle>
    {
        std::size_t operator()(const Bamboo::AssetHandle &handle) const noexcept
        {
            return static_cast<std::size_t>(handle.GetUUID());
        }
    };
}