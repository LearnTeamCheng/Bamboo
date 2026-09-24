#include "UUID.h"

#include <random>

namespace Bamboo
{
    static std::random_device s_RandomDevice;
    static std::mt19937_64 s_Engine(s_RandomDevice());
    static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

    UUID::UUID() 
    {
    }

    UUID::UUID(uint64_t uuid) : m_UUID(uuid)
    {
    }

    UUID UUID::Generate()
    {
        uint64_t value = 0;

        do
        {
            value = s_UniformDistribution(s_Engine);
        } while (value == 0);

        return UUID(value);
    }
}