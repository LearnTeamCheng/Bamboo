#pragma once
#include <functional>
#include <string>

#include "../Ref.h"
#include "Property.h"

namespace Bamboo
{
    class TypeInfo
    {
    public:
        using CreateFunc = std::function<void *(void)>;

        TypeInfo(const std::string &name, TypeInfo *parent = nullptr) : Name(name), Parent(parent) {}

        template <typename Class, typename T>
        Property *AddProperty(const std::string &name, T Class::*property)
        {
            auto prop = CreateScope<MemberProperty<Class, T>>(name, property);
            Property *raw = prop.get();
            Properties.push_back(std::move(prop));
            return raw;
        }

        std::vector<Property *> GetAllProperties() const
        {
            std::vector<Property *> result;
            if (Parent)
            {
                auto parentProps = Parent->GetAllProperties();
                result.insert(result.end(), parentProps.begin(), parentProps.end());
            }
            for (auto &prop : Properties)
            {
                result.push_back(prop.get());
            }
            return result;
        }

    public:
        std::string Name;
        TypeInfo *Parent;
        std::vector<Scope<Property>> Properties;
        CreateFunc Creator;
    };
};