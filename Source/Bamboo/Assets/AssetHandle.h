#pragma once
#include "../Core/UUID.h"

namespace Bamboo
{

    class AssetHandle
    {
    public:
        UUID GetId() const { return m_Id; }
        bool IsValid() const;

    private:
        UUID m_Id{};
    };
};