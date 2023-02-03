#include <gtest/gtest.h>
#include "additional_methods.hpp"


TEST(AddMethods, test1) {
  EXPECT_EQ (get_block_id(50),  0);
}


