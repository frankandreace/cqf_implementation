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
        generator.seed(time(NULL)); //time(NULL) //1
        bool verbose = false;
        normal_cqf = Cqf(18, 21, 20, verbose);
        small_cqf = Cqf(10,7, 20, verbose);
    }

    // void TearDown() override {}

    std::default_random_engine generator;
    std::uniform_int_distribution<uint64_t> distribution;

    Cqf normal_cqf;
    Cqf small_cqf;

    uint64_t max_shift_run_test = 22;
    uint64_t max_shift_query_test = 10;
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
        while (q_verif.count(quot) == 1 || (rem == 0) || verif.count(val) == 1)
        { // already seen key
            val = distribution(generator);
            val = (val & (mask_right(small_cqf.quotient_size + small_cqf.remainder_size)));
            quot = small_cqf.quotient(val);
            rem = small_cqf.remainder(val);
        }

        small_cqf.insert(val);
        small_cqf.display_vector();
        verif.insert({val, 1});
        q_verif.insert({quot, 1});
    }

    small_cqf.get_num_inserted_elements();
    small_cqf.compare_with_map(verif);

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
TEST_F(CqfTest, globalUse) {
    uint64_t val = distribution(generator);
    uint64_t val2 = distribution(generator);

    std::set<uint64_t> verif;

    for (uint64_t i = 0; i < (1ULL << max_shift_query_test) - 1; i++){
        val = distribution(generator);
        while (small_cqf.query(val) != 0) {val = distribution(generator);}
        small_cqf.insert(val,val%31);
        verif.insert(val);
        while (verif.find(val2) != verif.end()){ val2 = distribution(generator); }
        EXPECT_EQ(small_cqf.query(val), val%31);
        EXPECT_EQ(small_cqf.query(val2), 0);
    }
}

TEST_F(CqfTest, insertRDMoccs) {
    uint64_t val;
    std::map<uint64_t, uint64_t> verif; 

    //INSERT
    for (uint64_t i = 0; i < (1ULL << max_shift_run_test) - 1; i++){

        val = distribution(generator);
        val = (val & (mask_right(normal_cqf.quotient_size + normal_cqf.remainder_size)));

        while (verif.count(val) == 1) { //already seen key
            val = distribution(generator);
            val = (val & (mask_right(normal_cqf.quotient_size + normal_cqf.remainder_size)));    
        }  
        
        normal_cqf.insert(val, val % 31);

        if (val%31 != 0)
        {            
            verif.insert({val, val % 31});
        }
        
        //small_cqf.display_vector();
    }
    normal_cqf.print_offsets();
    std::map<uint64_t, uint64_t> out_filter = normal_cqf.enumerate();
    //small_cqf.compare_with_map(verif);
    normal_cqf.get_num_inserted_elements();
    EXPECT_EQ(out_filter, verif);

    //REMOVE
    //std::map<uint64_t,uint64_t>::iterator it;
    //for (it = verif.begin(); it != verif.end(); it++){
    //    cqf.remove((*it).first, (*it).second);
    //}
    //verif.clear();

    //EXPECT_EQ(cqf.enumerate(), verif);
}
