#pragma once
#include <string>
#include <unordered_map>
#include "TypeId.h"
#include "PropertyValue.h"

namespace Bamboo
{
    class TypeInfo;
    class Property
    {
    public:
        virtual ~Property() = default;
        const std::string &GetName() const { return m_Name; }
        TypeId GetValueType() const { return m_TypeId; }

        /// @brief 类型已知时的快速通道：直接返回成员地址。
        virtual void *GetPtr(void *instance) const = 0;

        /// @brief 通用读取：把字段值取出来装进 PropertyValue。
        ///
        /// 给"不关心具体类型"的消费者用（检查器、剪贴板、序列化）。
        /// 如果字段类型不在 PropertyValue 的备选列表里，会抛 std::bad_variant_access。
        virtual PropertyValue GetValue(const void *instance) const = 0;

        /// @brief 通用写入。传入的 variant 必须持有与字段相同的类型，否则抛 std::bad_variant_access。
        virtual void SetValue(void *instance, const PropertyValue &value) = 0;

        void SetMeta(const std::string &name, const std::string &value);

        TypeInfo *GetTypeInfo();

    protected:
        std::string m_Name;
        TypeId m_TypeId {typeid(void)};
        TypeInfo *m_TypeInfo = nullptr;
        std::unordered_map<std::string, std::string> m_MetaDatas;
    };

    template <typename Class, typename T>
    class MemberProperty : public Property
    {
    public:
        using MemberPtr = T Class::*;
        MemberProperty(const std::string &name, MemberPtr memberPtr): m_MemberPtr(memberPtr)
        {
            m_Name = name;
            m_TypeId = GetTypeId<T>();
        }

        void *GetPtr(void *instance) const override
        {
            Class *obj = static_cast<Class *>(instance);
            return &(obj->*m_MemberPtr); // 返回成员变量的地址
        }

        /// 只需要在这里写一次，所有被反射的类型自动获得通用读写能力 ——
        /// 新增字段类型只要它出现在 PropertyValue 里，零额外代码。
        PropertyValue GetValue(const void *instance) const override
        {
            return static_cast<const Class *>(instance)->*m_MemberPtr;
        }

        void SetValue(void *instance, const PropertyValue &value) override
        {
            static_cast<Class *>(instance)->*m_MemberPtr = std::get<T>(value);
        }

    private:
        MemberPtr m_MemberPtr; // 指向成员变量的指针
    };
};
