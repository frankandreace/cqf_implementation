#include "cqf.hpp"

using namespace std;

 
int main(int argc, char *argv[])
{
    Cqf small_cqf = Cqf(8, 9, true);
    std::map<uint64_t, uint64_t> verif;

    Rsqf small_rsqf = Rsqf(8, 9, true);
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

    for(const auto& elem : values_map)
    {
        std::cout << elem.first << " " << elem.second << "\n";

    }




    return 0;
}