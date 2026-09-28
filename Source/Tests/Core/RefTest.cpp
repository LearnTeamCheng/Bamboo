#include <gtest/gtest.h>
#include <vector>
#include "../Bamboo/Core/Memory/BRef.h"

namespace Bamboo
{
   class TestObject : public BRefCounted
   {
   public:
       static int DestructionCount;

       ~TestObject() override
       {
           ++DestructionCount;
       }
   };

   int TestObject::DestructionCount = 0;

   class TestObject2 : public TestObject{};

   class TestObject3 : public TestObject2{};

}


TEST(BRefTest, Create)
{
   auto ref = Bamboo::CreateBRef<Bamboo::TestObject>();

   ASSERT_TRUE(ref);
   EXPECT_EQ(ref->GetRefCount(), 1);
}


TEST(BRefTest, Copy)
{
   auto a = Bamboo::CreateBRef<Bamboo::TestObject>();

   EXPECT_EQ(a->GetRefCount(), 1);

   {
       auto b = a;

       EXPECT_EQ(a->GetRefCount(), 2);
       EXPECT_EQ(b->GetRefCount(), 2);
   }

   EXPECT_EQ(a->GetRefCount(), 1);
}


TEST(BRefTest, Move)
{
   auto a = Bamboo::CreateBRef<Bamboo::TestObject>();

   auto b = std::move(a);

   EXPECT_FALSE(a);
   EXPECT_TRUE(b);

   EXPECT_EQ(b->GetRefCount(), 1);
}


TEST(BRefTest, Destruction)
{
   Bamboo::TestObject::DestructionCount = 0;

   {
       auto ref = Bamboo::CreateBRef<Bamboo::TestObject>();

       EXPECT_EQ(Bamboo::TestObject::DestructionCount, 0);
   }

   EXPECT_EQ(Bamboo::TestObject::DestructionCount, 1);
}

TEST(BRefTest,Inheritance)
{

    auto ref = Bamboo::CreateBRef<Bamboo::TestObject2>();
    EXPECT_EQ(ref->GetRefCount(), 1);
    Bamboo::BRef<Bamboo::TestObject> ref2 = ref;

    EXPECT_EQ(ref->GetRefCount(), 2);
    EXPECT_EQ(ref2->GetRefCount(), 2);
}

TEST(BRefTest, MultiLevelInheritance) 
{
    auto ref3 = Bamboo::CreateBRef<Bamboo::TestObject3>();

    EXPECT_EQ(ref3->GetRefCount(), 1);

    Bamboo::BRef<Bamboo::TestObject2> ref2 = ref3;
    Bamboo::BRef<Bamboo::TestObject> ref1 = ref3;

    EXPECT_EQ(ref3->GetRefCount(), 3);
    EXPECT_EQ(ref2->GetRefCount(), 3);
    EXPECT_EQ(ref1->GetRefCount(), 3);
}

TEST(BRefTest, DerivedDestroyedThroughBase)
{
    Bamboo::TestObject::DestructionCount = 0;
    Bamboo::TestObject2::DestructionCount = 0;

    {
        auto derive = Bamboo::CreateBRef<Bamboo::TestObject2>();
        Bamboo::BRef<Bamboo::TestObject> base = derive;

        EXPECT_EQ(derive->GetRefCount(), 2);
        EXPECT_EQ(base->GetRefCount(), 2);
        
    }

    EXPECT_EQ(Bamboo::TestObject::DestructionCount, 1);
}


