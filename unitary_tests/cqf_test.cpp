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
        generator.seed(2); //time(NULL) //11
        small_cqf = Cqf(7, 4, true);
    }

    // void TearDown() override {}

    std::default_random_engine generator;
    std::uniform_int_distribution<uint64_t> distribution;

    Cqf small_cqf;

    uint64_t max_shift_run_test = 4;
};

/*
TEST_F(CqfTest, insert1value)
{
    uint64_t val = 0b000001000001;
    uint64_t count = 1ULL;
    std::map<uint64_t, uint64_t> verif;
    val = (val & (mask_right(small_cqf.quotient_size + small_cqf.remainder_size)));
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

TEST_F(CqfTest, insert1occ_per_quotient)
{
    uint64_t val;
    uint64_t quot;
    uint64_t rem;
    std::map<uint64_t, uint64_t> verif;
    std::map<uint64_t, uint64_t> q_verif;

    // INSERT
    for (uint64_t i = 0; i < (1ULL << max_shift_run_test) - 1; i++)
    {
        val = distribution(generator);
        val = (val & (mask_right(small_cqf.quotient_size + small_cqf.remainder_size)));
        quot = small_cqf.quotient(val);
        rem = small_cqf.remainder(val);
        while (q_verif.count(quot) == 1 || (rem == 0))
        { // already seen key
            val = distribution(generator);
            val = (val & (mask_right(small_cqf.quotient_size + small_cqf.remainder_size)));
            quot = small_cqf.quotient(val);
            rem = small_cqf.remainder(val);
        }

        small_cqf.insert(val);
        //small_cqf.display_vector();
        verif.insert({val, 1});
        q_verif.insert({quot, 1});
    }

    small_cqf.get_num_inserted_elements();
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

TEST_F(CqfTest, insert1occ_per_hash)
{
    uint64_t val;
    std::map<uint64_t, uint64_t> verif;

    // INSERT
    for (uint64_t i = 0; i < (1ULL << max_shift_run_test) - 1; i++)
    {
        val = distribution(generator);
        val = (val & (mask_right(small_cqf.quotient_size + small_cqf.remainder_size)));
        
        while (verif.count(val) == 1)
        { // already seen key
            val = distribution(generator);
            val = (val & (mask_right(small_cqf.quotient_size + small_cqf.remainder_size)));
        }
        small_cqf.insert(val);
        //small_cqf.display_vector();
        verif.insert({val, 1});
    }

    small_cqf.get_num_inserted_elements();
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

*/




TEST_F(CqfTest, insertRDMoccs) {
    uint64_t val;
    std::map<uint64_t, uint64_t> verif; 

    //INSERT
    for (uint64_t i = 0; i < (1ULL << max_shift_run_test) - 1; i++){

        val = distribution(generator);
        val = (val & (mask_right(small_cqf.quotient_size + small_cqf.remainder_size)));

        while (verif.count(val) == 1) { //already seen key
            val = distribution(generator);
            val = (val & (mask_right(small_cqf.quotient_size + small_cqf.remainder_size)));    
        }  
        
        small_cqf.insert(val, val % 31);

        if (val%31 != 0)
        {            
            verif.insert({val, val % 31});
        }
        
        small_cqf.display_vector();
    }

    small_cqf.get_num_inserted_elements();
    EXPECT_EQ(small_cqf.enumerate(), verif);

    //REMOVE
    //std::map<uint64_t,uint64_t>::iterator it;
    //for (it = verif.begin(); it != verif.end(); it++){
    //    cqf.remove((*it).first, (*it).second);
    //}
    //verif.clear();

    //EXPECT_EQ(cqf.enumerate(), verif);
}
