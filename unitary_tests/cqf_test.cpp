#include <gtest/gtest.h>
#include "filter.hpp"
#include <random>

class CqfTest : public ::testing::Test {
 protected:
  void SetUp() override {
    generator.seed(time(NULL));
    
    filter1 = Cqf(4);
  }

  // void TearDown() override {}

  std::default_random_engine generator;
  std::uniform_int_distribution<uint64_t> distribution;

  Cqf filter1;
};


TEST_F(CqfTest, QuotientSize) {
    EXPECT_EQ(filter1.get_quot_size(), 19);
}

TEST_F(CqfTest, globalUse) {
    uint64_t val = distribution(generator);
    uint64_t val2 = distribution(generator);

    while (val == val2){ val2 = distribution(generator); }
    filter1.insert(val);
    EXPECT_EQ(filter1.query(val), 1);
    EXPECT_EQ(filter1.query(val2), 0);
}




