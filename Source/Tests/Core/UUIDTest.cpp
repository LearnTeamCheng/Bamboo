#include <gtest/gtest.h>

#include "../Bamboo/Core/UUID.h"

TEST(UUIDTest, GenerateUUID)
{
    Bamboo::UUID uuid = Bamboo::UUID::Generate();
    EXPECT_NE(uuid, 0);
}