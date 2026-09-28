#pragma once
#include "TypeRegister.h"
namespace Bamboo
{

//@brief 定义一个类型
#define DECLARE_TYPE(Type, ParentType) \
public:                                \
    static TypeInfo s_TypeInfo;        \
    static void RegisterType();        \
    static TypeInfo *StaticType();     \
    virtual TypeInfo *GetType() const { return &s_TypeInfo; }

//@brief 实现一个类型
#define IMPLEMENT_TYPE(Type, ParentType)                                                   \
    TypeInfo Type::s_TypeInfo{#Type, &ParentType::s_TypeInfo};                             \
    TypeInfo *Type::StaticType() { return &s_TypeInfo; }                                   \
    static struct Type##AutoRegister                                                       \
    {                                                                                      \
        Type##AutoRegister()                                                               \
        {                                                                                  \
            Type::RegisterType();                                                          \
            Type::s_TypeInfo.Creator = []() { return new Type(); }; \
            TypeRegister::Instance().RegisterType(&Type::s_TypeInfo);                      \
        }                                                                                  \
    } s_##Type##AutoRegister;

//@brief 定义一个根类型
#define IMPLEMENT_ROOT_TYPE(Type)                                     \
    TypeInfo Type::s_TypeInfo{#Type, nullptr};                        \
    TypeInfo *Type::StaticType() { return &s_TypeInfo; }              \
    static struct Type##AutoRegister                                  \
    {                                                                 \
        Type##AutoRegister()                                          \
        {                                                             \
            Type::RegisterType();                                     \
            TypeRegister::Instance().RegisterType(&Type::s_TypeInfo); \
        }                                                             \
    } s_##Type##AutoRegister;

    //@brief 定义一个属性
#define BEGIN_PROPERTIES(Type) \
    void Type::RegisterType()  \
    {                          \
        using ThisClass = Type;

#define PROPERTIES(name, member) \
    s_TypeInfo.AddProperty<ThisClass>(name, &ThisClass::member);

#define END_PROPERTIES() \
    }

};
