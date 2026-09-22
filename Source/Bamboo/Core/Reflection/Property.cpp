#include "Property.h"
#include "TypeInfo.h"

namespace Bamboo
{
    TypeInfo *Property::GetTypeInfo()
    {
        return m_TypeInfo;
    }

    void Property::SetMeta(const std::string &name, const std::string &value)
    {
        m_MetaDatas[name] = value;
    }
};