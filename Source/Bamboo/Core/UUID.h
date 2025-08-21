#pragma once
/// @file UUID.h

#include <cstdint>
#include <iostream>
namespace Bamboo {
    class UUID {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID& other) = default;
        operator uint64_t() const { return m_UUID; }
    private:
        uint64_t m_UUID;
    };
};


namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<Bamboo::UUID>
	{
		std::size_t operator()(const Bamboo::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

}
