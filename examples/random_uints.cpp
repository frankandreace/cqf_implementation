/*
	Generates n random integers and feed the cqf with them.
*/

#include <iostream>
#include <sstream>
#include <random>
#include <ctime>
#include <getopt.h>
#include "filter.hpp"


using namespace std;


void parse_cmd(int argc, char *argv[], size_t & n, uint64_t & seed, bool & verbose);


int main (int argc, char * argv[]) {
	// Argument prep and parsing
	size_t n = 0;
	uint64_t seed = time(NULL);
	bool verbose = false;
	parse_cmd(argc, argv, n, seed, verbose);

	// Creation of the cqf (size is MB)
	uint64_t quotien_size = 1;
	Cqf cqf(quotien_size);

	// uint64 generators
	default_random_engine generator;
	generator.seed(seed);
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


void print_cmd() {
	cerr << "cmd line: random_uints -n <value> [-s <rnd_seed>] [-v]" << endl;
	cerr << "   -n <val>: Number of random uint to insert." << endl;
	cerr << "   -s <val>: uint64_t random seed." << endl;
	cerr << "   -v: verbose flag." << endl;
}


void parse_cmd(int argc, char *argv[], size_t & n, uint64_t & seed, bool & verbose) {
	int opt;

    while ((opt = getopt(argc, argv, "hn:s:v")) != -1) {
        switch (opt) {
        	case 'h':
        		print_cmd();
        		exit(0);
            case 'n':
                n = atoi(optarg);
                break;
            case 's': {
	    		istringstream iss(optarg);
                iss >> seed;
            	}
                break;
            case 'v':
                verbose = true;
                break;
            default:
            	print_cmd();
            	exit(1);
        }
    }

    if (n == 0) {
    	print_cmd();
    	exit(1);
    }
}
