#include <gtest/gtest.h>
#include "filter.hpp"

class CqfTest : public ::testing::Test {
 protected:
  void SetUp() override {
     filter1 = Cqf(4);
  }

  // void TearDown() override {}

  Cqf filter1;
};


TEST_F(CqfTest, QuotientSize) {
    EXPECT_EQ(filter1.get_quot_size(), 19);
}



