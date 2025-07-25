#pragma once
/// @file UUID.h


namespace Bamboo {
    class UUID {
        public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID& other) = default;
        operator uint64_t() const { return m_UUID; }
    private:
        uint64_t m_UUID;
    }
}