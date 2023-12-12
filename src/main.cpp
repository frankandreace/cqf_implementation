#include "cqf.hpp"
#include <random>

using namespace std;

 
int main(int argc, char *argv[])
{
    std::default_random_engine generator;
    std::uniform_int_distribution<uint64_t> distribution;
    
    Cqf small_cqf = Cqf(8, 9, true);


    std::map<uint64_t, uint64_t> verif;

    /*Rsqf small_rsqf = Rsqf(8, 9, true);
    small_rsqf.insert(0b000100000000);
    small_rsqf.insert(0b000110000000);
    small_rsqf.insert(0b001100000000);
    small_rsqf.insert(0b001110000000);
    small_rsqf.display_vector();
    unordered_set<uint64_t> values = small_rsqf.enumerate();
    for (auto it = values.cbegin(); it != values.cend(); it++) {
        std::cout << "val: " << *it << ' ';
    }
    cout << endl << endl << endl;
    */

    // INSERT
    uint64_t val = 0b0010100000011;
    uint64_t count = 85ULL;
    cout << "Inserting " << val << " with quotient " << small_cqf.quotient(val) << " remainder " << small_cqf.remainder(val) << " and count " << count << endl;
    small_cqf.insert(val,count);
    verif.insert({val, count});
    small_cqf.display_vector();

    val = 0b0010000000011;
    count = 1421ULL;
    cout << "Inserting " << val << " with quotient " << small_cqf.quotient(val) << " remainder " << small_cqf.remainder(val) << " and count " << count << endl;
    small_cqf.insert(val,count);
    verif.insert({val, count});
    small_cqf.display_vector();

    val = 0b0011000000011;
    count = 2ULL;
    cout << "Inserting " << val << " with quotient " << small_cqf.quotient(val) << " remainder " << small_cqf.remainder(val) << " and count " << count << endl;
    small_cqf.insert(val,count);
    verif.insert({val, count});
    small_cqf.display_vector();

    val = 0b0010000000110;
    count = 2ULL;
    cout << "Inserting " << val << " with quotient " << small_cqf.quotient(val) << " remainder " << small_cqf.remainder(val) << " and count " << count << endl;
    small_cqf.insert(val,count);
    verif.insert({val, count});
    small_cqf.display_vector();

    val = 0b100000000100;
    count = 54ULL;
    cout << "Inserting " << val << " with quotient " << small_cqf.quotient(val) << " remainder " << small_cqf.remainder(val) << " and count " << count << endl;
    small_cqf.insert(val,count);
    verif.insert({val, count});
    small_cqf.display_vector();


    val = 0b000110000000;
    count = 11ULL;
    cout << "Inserting " << val << " with quotient " << small_cqf.quotient(val) << " remainder " << small_cqf.remainder(val) << " and count " << count << endl;
    small_cqf.insert(val,count);
    verif.insert({val, count});
    small_cqf.display_vector();



    verif.insert({val, count});

    map<uint64_t, uint64_t> values_map = small_cqf.enumerate();
    std::cout << "VERIFICATION" << std::endl;
    for(const auto& elem : values_map)
    {
        std::cout << elem.first << " " << elem.second << "\n";
    }
    std::cout << std::endl;
    for(const auto& elem : verif)
    {
        std::cout << elem.first << " " << elem.second << "\n";
    }


    Cqf big_cqf = Cqf(25, 18, true);
    uint64_t max_shift_run_test = 5;


    verif.clear();
    generator.seed(0); // time(NULL)
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
        small_cqf.display_vector();
        verif.insert({val, 1});
    }
    if ( verif.size() == small_cqf.enumerate().size()) {
        cout << "works~" << endl;
    }
    return 0;
}