#include "counter_encoder.hpp"

using namespace std;


class Counter_encoder
{
public:
    Counter_encoder::Counter_encoder(uint64_t num_values){
        counter = std::vector<uint64_t>(num_values);
        position = 0;
    }

    uint64_t Counter_encoder::pop_front(){
        return counter[position];
    }

    void Counter_encoder::push_back(uint64_t value){
        counter[position] = value;
        position = ++position % counter.size();
    }

private:
    std::vector<uint64_t> counter;
    uint64_t position;
};