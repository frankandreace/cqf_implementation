#include "cqf.hpp"

using namespace std;

 
int main(int argc, char *argv[])
{
    Cqf small_cqf = Cqf(8, 5, true);
    uint64_t val = 0b0010100000011;
    uint64_t count = 33ULL;
    std::map<uint64_t, uint64_t> verif;
    cout << "quotient of val " << small_cqf.quotient(val) << endl;
    cout << "reminder of val " << small_cqf.remainder(val) << endl;

    // INSERT
    small_cqf.insert(val,count);
    verif.insert({val, count});

    val = 0b0010000000011;
    count = 11ULL;
    small_cqf.insert(val,count);
    verif.insert({val, count});

    val = 0b0011000000011;
    count = 2ULL;
    small_cqf.insert(val,count);
    verif.insert({val, count});




    verif.insert({val, count});

    map<uint64_t, uint64_t> values_map = small_cqf.enumerate();

    for(const auto& elem : values_map)
    {
        std::cout << elem.first << " " << elem.second << "\n";
    }

    return 0;
}