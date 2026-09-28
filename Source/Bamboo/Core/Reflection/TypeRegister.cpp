#include "TypeRegister.h"

namespace Bamboo
{

    TypeRegister &TypeRegister::Instance()
    {
        static TypeRegister instance;
        return instance;
    }

    void TypeRegister::RegisterType(TypeInfo *typeInfo)
    {
        m_TypeInfos[typeInfo->Name] = typeInfo;
    }

    TypeInfo *TypeRegister::GetTypeInfo(const std::string &typeName)
    {
        auto it = m_TypeInfos.find(typeName);
        if (it != m_TypeInfos.end())
        {
            return it->second;
        }
        return nullptr;
    }

    void TypeRegister::AddDeferred(std::function<void()> func)
    {
        m_Deferred.push_back(func);
    }

    void TypeRegister::ProcessDeferred()
    {
        for (auto &func : m_Deferred)
        {
            func();
        }
        m_Deferred.clear();
    }

    const std::unordered_map<std::string, TypeInfo *> &TypeRegister::GetTypes() const
    {
        return m_TypeInfos;
    }

};