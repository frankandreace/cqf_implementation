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


TEST_F(CqfTest, offset1_blockOverflow) {
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

/* TEST_F(CqfTest, offset2_runOverflowsBy1) {
    //bugs because of offset being 0 when there is an element in pos 0 and nothing in 1(we dont know)
    small_cqf.insert((3ULL<<31) + 64);

    small_cqf.insert((3ULL<<30) + 63);
    small_cqf.insert((3ULL<<30) + 63);

    EXPECT_EQ(small_cqf.get_remainder(64), 3ULL<<30);
} */

TEST_F(CqfTest, offset3_insertshift0) {
    small_cqf.insert((1<<11)+ 64); EXPECT_EQ(small_cqf.get_offset_word(1), 0);
    small_cqf.insert((1<<11)+ 64); EXPECT_EQ(small_cqf.get_offset_word(1), 1);

    small_cqf.remove((1<<11)+ 64); EXPECT_EQ(small_cqf.get_offset_word(1), 0);
    small_cqf.remove((1<<11)+ 64); EXPECT_EQ(small_cqf.get_offset_word(1), 0);
    small_cqf.remove((1<<11)+ 64); EXPECT_EQ(small_cqf.get_offset_word(1), 0);
}


TEST_F(CqfTest, offset4_multiBlockRun) {
    for (int i = 0; i < 128; i++){ usual_cqf.insert((1ULL<<32)+ 100); }
    EXPECT_EQ(usual_cqf.get_offset_word(1), 0);
    EXPECT_EQ(usual_cqf.get_offset_word(2), 99);
    EXPECT_EQ(usual_cqf.get_offset_word(3), 99-64);    

    for (int i = 0; i < 50; i++){ usual_cqf.remove((1ULL<<32)+ 100); }
    EXPECT_EQ(usual_cqf.get_offset_word(1), 0);
    EXPECT_EQ(usual_cqf.get_offset_word(2), 49);
    EXPECT_EQ(usual_cqf.get_offset_word(3), 0); 
}

TEST_F(CqfTest, offset5_complexCase) {
    for (int i = 0; i < 60; i++){ usual_cqf.insert((1ULL<<32)+ 20); } 
    //run of quot 20, starts @20 ends up @79
    for (int i = 0; i < 69; i++){ usual_cqf.insert((1ULL<<32)+ 40); } 
    //run of quot 40, starts @80 ends up @148
    usual_cqf.insert((1ULL<<32)+ 149);
    //run of quot 149, starts @149 ends up @149 
    //(boundary shift deletion in case we delete in 2nd run, because can't be shifted)
    for (int i = 0; i < 11; i++){ usual_cqf.insert((1ULL<<32)+ 150); }
    //run of quot 150, starts @150 ends up @160 (FUS == 161)

    EXPECT_EQ(usual_cqf.find_boundary_shift_deletion(100, 161), 148); 
    EXPECT_EQ(usual_cqf.get_offset_word(0), 0); 
    EXPECT_EQ(usual_cqf.get_offset_word(1), 148-64); 
    EXPECT_EQ(usual_cqf.get_offset_word(2), 148-128); 

    usual_cqf.remove((1ULL<<32)+ 155); 
    EXPECT_EQ(usual_cqf.get_offset_word(2), 148-128); 
    for (int i = 0; i < 20; i++){ usual_cqf.remove((1ULL<<32)+ 20); } 
    EXPECT_EQ(usual_cqf.get_offset_word(0), 0); 
    EXPECT_EQ(usual_cqf.get_offset_word(1), 128-64); 
    EXPECT_EQ(usual_cqf.get_offset_word(2), 128-128); 
}

/* TEST_F(CqfTest, offset6_toricity) {
    for (int i = 0; i < 100; i++){ small_cqf.insert((1ULL<<32)+ 40); }

    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);


    EXPECT_EQ(small_cqf.get_offset_word(0), 11); 
    EXPECT_EQ(small_cqf.get_offset_word(1), 75); 
} */







/* TEST_F(CqfTest, enumerate1) {
    //bugs because of offset2_runOverflowsBy1
    std::unordered_set<uint64_t> verif;
    
    small_cqf.insert((3ULL<<30) + 35); verif.insert((3ULL<<30) + 35);
    small_cqf.insert((3ULL<<31) + 35); verif.insert((3ULL<<31) + 35);
    small_cqf.insert((3ULL<<25) + 35); verif.insert((3ULL<<25) + 35);

    small_cqf.insert((3ULL<<30) + 64); verif.insert((3ULL<<30) + 64);

    small_cqf.insert((3ULL<<30) + 63); verif.insert((3ULL<<30) + 63);
    small_cqf.insert((3ULL<<30) + 63); verif.insert((3ULL<<30) + 63);

    std::cout << small_cqf.block2string(0) << "\n" << small_cqf.block2string(1);


    EXPECT_EQ(small_cqf.enumerate(), verif);
} */


TEST_F(CqfTest, enumerate2) {
    std::unordered_set<uint64_t> verif;
    for (int i=0; i<100; i++){
        uint64_t val = distribution(generator);
        verif.insert(val);
        usual_cqf.insert(val);
    }
    EXPECT_EQ(usual_cqf.enumerate(), verif);
}
