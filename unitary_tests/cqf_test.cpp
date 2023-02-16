#include <gtest/gtest.h>
#include "filter.hpp"
#include <random>

using namespace std;

class CqfTest : public ::testing::Test {
 protected:
  void SetUp() override {
    generator.seed(time(NULL));
    
    usual_cqf = Cqf(4);
    small_cqf = Cqf(7, 64-7, false);
  }

  // void TearDown() override {}

  std::default_random_engine generator;
  std::uniform_int_distribution<uint64_t> distribution;

  Cqf usual_cqf;
  Cqf small_cqf;
};


TEST_F(CqfTest, QuotientSize) {
    EXPECT_EQ(usual_cqf.get_quot_size(), 19);
}

TEST_F(CqfTest, globalUse) {
    uint64_t val = distribution(generator);
    uint64_t val2 = distribution(generator);

    while (val == val2){ val2 = distribution(generator); }
    usual_cqf.insert(val);
    EXPECT_EQ(usual_cqf.query(val), 1);
    EXPECT_EQ(usual_cqf.query(val2), 0);
}


/* TEST_F(CqfTest, get_run_boundaries) {
    std::pair<uint64_t,uint64_t> compare(126, 2);
    
    small_cqf.insert((2ULL<<30)+126);
    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);
    small_cqf.insert((2ULL<<31)+126);
    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);
    small_cqf.insert((2ULL<<32)+126);
    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);
    small_cqf.insert((2ULL<<33)+126);
    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);
    small_cqf.insert((2ULL<<34)+126);
    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);
    EXPECT_EQ(small_cqf.get_run_boundaries(126), compare);
} */



TEST_F(CqfTest, get_runstart) {
    small_cqf.insert((2ULL<<30)+64);
    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);
    EXPECT_EQ(small_cqf.get_runstart(64), 64);
    small_cqf.insert((2ULL<<31)+63);

    small_cqf.insert((2ULL<<32)+126);
    small_cqf.insert((2ULL<<33)+126);
    small_cqf.insert((2ULL<<34)+126);
    EXPECT_EQ(small_cqf.get_runstart(64), 64);
}



TEST_F(CqfTest, insert_pos_0) {
    small_cqf.insert((2ULL << 30) + 90); 
    small_cqf.insert(2ULL << 31); 
    //std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);
    EXPECT_EQ(small_cqf.get_remainder(0), 33554432);
}


TEST_F(CqfTest, offset1) {
    for (int i = 0; i < 5; i++) { small_cqf.insert((1<<11)+ 62); }

    //std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);
    EXPECT_EQ(small_cqf.get_offset_word(1), 2);

    small_cqf.remove((1<<11)+ 62);
    EXPECT_EQ(small_cqf.get_offset_word(1), 1);
    small_cqf.remove((1<<11)+ 62);
    EXPECT_EQ(small_cqf.get_offset_word(1), 0);
    small_cqf.remove((1<<11)+ 62);
    EXPECT_EQ(small_cqf.get_offset_word(1), 0);
}

TEST_F(CqfTest, offset2) {
    small_cqf.insert((1<<11)+ 64); EXPECT_EQ(small_cqf.get_offset_word(1), 0);
    small_cqf.insert((1<<11)+ 64); EXPECT_EQ(small_cqf.get_offset_word(1), 1);

    small_cqf.remove((1<<11)+ 64); EXPECT_EQ(small_cqf.get_offset_word(1), 0);
    small_cqf.remove((1<<11)+ 64); EXPECT_EQ(small_cqf.get_offset_word(1), 0);
    small_cqf.remove((1<<11)+ 64); EXPECT_EQ(small_cqf.get_offset_word(1), 0);
}


TEST_F(CqfTest, offset3) {
    for (int i = 0; i < 128; i++){ usual_cqf.insert((1ULL<<32)+ 100); }
    EXPECT_EQ(usual_cqf.get_offset_word(1), 0);
    EXPECT_EQ(usual_cqf.get_offset_word(2), 99);
    EXPECT_EQ(usual_cqf.get_offset_word(3), 99-64);    

    for (int i = 0; i < 50; i++){ usual_cqf.remove((1ULL<<32)+ 100); }
    EXPECT_EQ(usual_cqf.get_offset_word(1), 0);
    EXPECT_EQ(usual_cqf.get_offset_word(2), 49);
    EXPECT_EQ(usual_cqf.get_offset_word(3), 0); 
}


