#pragma once
#include <string>
#include <unordered_map>
#include "TypeId.h"

namespace Bamboo
{
    class TypeInfo;
    class Property
    {
    public:
        virtual ~Property() = default;
        const std::string &GetName() const { return m_Name; }
        TypeId GetValueType() const { return m_TypeId; }

        void SetMeta(const std::string &name, const std::string &value);

        TypeInfo *GetTypeInfo();

    protected:
        std::string m_Name;
        TypeId m_TypeId;
        TypeInfo *m_TypeInfo;
        std::unordered_map<std::string, std::string> m_MetaDatas;
    };

    template <typename Class, typename PropertyType>
    class MemberProperty : public Property
    {
    public:
        using MemberPtr = PropertyType Class::*;
        MemberProperty(const std::string &name, MemberPtr memberPtr): m_MemberPtr(memberPtr)
        {
            m_Name = name;
            m_TypeId = GetTypeId<PropertyType>();
        }

        void *GetPtr(void *instance)
        {
            Class *obj = static_cast<Class *>(instance);
            return &obj->*m_MemberPtr; // 返回成员变量的地址
        }

    private:
        MemberPtr m_MemberPtr; // 指向成员变量的指针
    };
};