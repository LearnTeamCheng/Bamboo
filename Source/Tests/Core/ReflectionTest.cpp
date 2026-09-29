#include <gtest/gtest.h>
#include <string>
#include <typeindex>
#include "../Bamboo/Core/Reflection/TypeInfo.h" // Include the TypeRegister header
#include "../Bamboo/Core/Reflection/ReflectionMacros.h"
namespace Bamboo
{
    class TestProject
    {

        DECLARE_TYPE(TestProject,nullptr)
    };

    IMPLEMENT_ROOT_TYPE(TestProject)
    BEGIN_PROPERTIES(TestProject)
    END_PROPERTIES()

    class TestProjectChild : public TestProject
    {
        DECLARE_TYPE(TestProjectChild, TestProject)

        public:
            float x{ 0.0f };
            float y{ 0.0f };
            float z{ 0.0f };
    };
    
    IMPLEMENT_TYPE(TestProjectChild, TestProject)
    BEGIN_PROPERTIES(TestProjectChild)
    PROPERTIES("X", x)
    PROPERTIES("Y", y)
    PROPERTIES("Z", z)
    END_PROPERTIES()


    class TestObject1 {
        DECLARE_TYPE(TestProject,nullptr)
        public:
        int id {0};
    };

    IMPLEMENT_ROOT_TYPE(TestObject1)
    BEGIN_PROPERTIES(TestObject1)
    PROPERTIES("ID", id)
    END_PROPERTIES()

    class TestObject2 :public TestProjectChild
    {
        DECLARE_TYPE(TestProject2, TestProjectChild)

        public:
            float x{ 0.0f };
            float y{ 0.0f };
            float z{ 0.0f };
    };
    
    IMPLEMENT_TYPE(TestObject2, TestObject1)
    BEGIN_PROPERTIES(TestObject2)
    PROPERTIES("X", x)
    PROPERTIES("Y", y)
    PROPERTIES("Z", z)
    END_PROPERTIES()
    

};

TEST(ReflectionTest, TestProjectReflection)
{
    Bamboo::TestProjectChild child;
    EXPECT_EQ(child.StaticType()->Name, "TestProjectChild");

    auto properties = child.s_TypeInfo.GetAllProperties();
    child.x = 200;

    EXPECT_EQ(properties[0]->GetPtr(&child), &child.x);

}

TEST(ReflectionTest, TestFieldReflection)
{
    Bamboo::TestProjectChild child;

    auto properties = child.s_TypeInfo.GetAllProperties();
    child.x = 200;

    // EXPECT_EQ(properties[0]->GetPtr(&child), &child.x);
    EXPECT_EQ(properties[0]->GetName(), "X");
}

// 自动注册依赖静态初始化 —— 最容易"看起来写了、运行时其实没做"的地方。
// 这条测试把"反射能不能用"从"编译通过"变成"运行时验证过"。
TEST(ReflectionTest, AutoRegistrationWorks)
{
    auto *info = Bamboo::TypeRegister::Instance().GetTypeInfo("TestProjectChild");
    ASSERT_NE(info, nullptr)
        << "IMPLEMENT_TYPE 的自动注册没有生效：TypeRegister 里查不到 TestProjectChild。"
        << "检查 AutoRegister 那个静态对象是否真的在静态初始化期被构造。";
    EXPECT_EQ(info->Name, "TestProjectChild");

    // 反序列化要靠 Creator 建实例，所以必须验证它被设置了
    ASSERT_TRUE(static_cast<bool>(info->Creator)) << "TypeInfo::Creator 没有被设置";
    void *created = info->Creator();
    ASSERT_NE(created, nullptr);
    delete static_cast<Bamboo::TestProjectChild *>(created);
}

// 验证属性的类型信息 —— 检查器和序列化要靠它决定"怎么画 / 怎么写"
TEST(ReflectionTest, PropertyTypeIsRecorded)
{
    Bamboo::TestProjectChild child;
    auto properties = child.s_TypeInfo.GetAllProperties();

    ASSERT_GE(properties.size(), 3u);
    EXPECT_EQ(properties[0]->GetValueType(), std::type_index(typeid(float)));
    EXPECT_EQ(properties[0]->GetName(), "X");
    EXPECT_EQ(properties[1]->GetName(), "Y");
    EXPECT_EQ(properties[2]->GetName(), "Z");
}

// 通用读写往返：序列化器真正依赖的路径
TEST(ReflectionTest, PropertyValueRoundTrip)
{
    using Bamboo::PropertyValue;

    Bamboo::TestProjectChild child;
    auto properties = child.s_TypeInfo.GetAllProperties();
    ASSERT_GE(properties.size(), 3u);

    // 1) 写进字段 → 通用读取应拿到同一个值
    child.x = 3.5f;
    child.y = -1.25f;
    PropertyValue valueX = properties[0]->GetValue(&child);
    PropertyValue valueY = properties[1]->GetValue(&child);
    ASSERT_TRUE(std::holds_alternative<float>(valueX));
    EXPECT_FLOAT_EQ(std::get<float>(valueX), 3.5f);
    EXPECT_FLOAT_EQ(std::get<float>(valueY), -1.25f);

    // 2) 通用写入到一个全新实例
    Bamboo::TestProjectChild other;
    properties[0]->SetValue(&other, valueX);
    properties[1]->SetValue(&other, valueY);

    EXPECT_FLOAT_EQ(other.x, 3.5f);
    EXPECT_FLOAT_EQ(other.y, -1.25f);

    // 3) 再读出来，值应保持一致（这就是序列化往返的核心）
    EXPECT_FLOAT_EQ(std::get<float>(properties[0]->GetValue(&other)), 3.5f);
    EXPECT_FLOAT_EQ(std::get<float>(properties[1]->GetValue(&other)), -1.25f);

    // 4) 通过 GetPtr 写、通过 GetValue 读，两条通道必须一致
    *static_cast<float *>(properties[2]->GetPtr(&other)) = 7.0f;
    EXPECT_FLOAT_EQ(std::get<float>(properties[2]->GetValue(&other)), 7.0f);
}

// 验证继承关系：子类的属性应该包含父类的属性
TEST(ReflectionTest, InheritedProperties)
{
    Bamboo::TestObject2 child;
    auto properties = child.s_TypeInfo.GetAllProperties();
    
    EXPECT_EQ(properties.size(), 4u);
    EXPECT_EQ(properties[0]->GetName(), "ID");
}
