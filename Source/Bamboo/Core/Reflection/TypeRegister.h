#pragma once
#include "TypeInfo.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
namespace Bamboo
{
    class TypeRegister
    {
    public:
        static TypeRegister &Instance();

        void RegisterType(TypeInfo *typeInfo);
        TypeInfo *GetTypeInfo(const std::string &typeName);

        template <typename T>
        TypeInfo *GetType()
        {
            auto it = m_TypeIdMap.find(GetTypeId<T>());
            if (it != m_TypeIdMap.end())
            {
                return it->second;
            }
            return nullptr;
        }

        template <typename T>
        void RegisterType(TypeInfo *typeInfo)
        {
            m_TypeIdMap[GetTypeId<T>()] = typeInfo;
            m_TypeInfos[typeInfo->Name] = typeInfo;
        }

        const std::unordered_map<std::string, TypeInfo *> &GetTypes() const;
        void AddDeferred(std::function<void()> func);
        void ProcessDeferred();

    private:
        std::unordered_map<std::string, TypeInfo *> m_TypeInfos;
        std::unordered_map<std::type_index, TypeInfo *> m_TypeIdMap;
        std::vector<std::function<void()>> m_Deferred;
    };
};