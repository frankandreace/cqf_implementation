/*
	Generates n random integers and feed the cqf with them.
*/

#include <iostream>
#include <random>
#include <getopt.h>
#include "filter.hpp"


using namespace std;


void parse_cmd(int argc, char *argv[], size_t & n);


int main (int argc, char * argv[]) {
	size_t n = 0;
	parse_cmd(argc, argv, n);

	// Creation of the cqf (size is MB)
	uint64_t quotien_size = 1;
	Cqf cqf(quotien_size);

	// uint64 generators
	default_random_engine generator;
	uniform_int_distribution<uint64_t> distribution;

	// Add the uints one by one into the cqf
	for (size_t i=0 ; i<n ; i++) {
		uint64_t val = distribution(generator);
		cout << i << " " << val << endl;
		cqf.insert(val);
		cout << endl;
	}

	return 0;
}


void parse_cmd(int argc, char *argv[], size_t & n) {
	int opt;

    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            default:
            	cerr << "cmd line: random_uints -n <value>" << endl;
            	exit(1);
        }
    }
}
