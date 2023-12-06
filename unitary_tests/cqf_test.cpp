#include <gtest/gtest.h>

#include "rsqf.hpp"
#include "cqf.hpp"
#include <random>

using namespace std;

class CqfTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        generator.seed(time(NULL));
        small_cqf = Cqf(7, 5, true);
    }

    // void TearDown() override {}

    std::default_random_engine generator;
    std::uniform_int_distribution<uint64_t> distribution;

    Cqf small_cqf;
};


TEST_F(CqfTest, insert1value)
{
    uint64_t val = 000001000001;
    uint64_t count = 1ULL;
    std::map<uint64_t, uint64_t> verif;

    // INSERT
    small_cqf.insert(val,count);
    verif.insert({val, count});

    EXPECT_EQ(small_cqf.enumerate(), verif);

    // REMOVE
    //std::map<uint64_t, uint64_t>::iterator it;
    //for (it = verif.begin(); it != verif.end(); it++)
    //{
    //    small_cqf.remove((*it).first);
    //}
    //verif.clear();

    //EXPECT_EQ(small_cqf.enumerate(), verif);
}




TEST_F(CqfTest, insert1occs)
{
    uint64_t val;
    std::map<uint64_t, uint64_t> verif;

    // INSERT
    for (uint64_t i = 0; i < (1ULL << 17) - 1; i++)
    {
        val = distribution(generator);
        while (verif.count(val) == 1)
        { // already seen key
            val = distribution(generator);
        }

        small_cqf.insert(val);
        verif.insert({val, 1});
    }

    EXPECT_EQ(small_cqf.enumerate(), verif);

    // REMOVE
    //std::map<uint64_t, uint64_t>::iterator it;
    //for (it = verif.begin(); it != verif.end(); it++)
    //{
    //    small_cqf.remove((*it).first);
    //}
    //verif.clear();

    //EXPECT_EQ(small_cqf.enumerate(), verif);
}
