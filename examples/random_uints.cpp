/*
	Generates n random integers and feed the cqf with them.
*/

#include <iostream>
#include <sstream>
#include <random>
#include <ctime>
#include <getopt.h>
#include <unordered_set>
#include "filter.hpp"


using namespace std;


void parse_cmd(int argc, char *argv[], size_t & n, size_t & filter_size, uint64_t & seed, bool & verbose, bool & debug);


int main (int argc, char * argv[]) {
	// Argument prep and parsing
	size_t n = 0;
	size_t filter_size = 4;
	uint64_t seed = time(NULL);
	bool verbose = false;
	bool debug = false;
	parse_cmd(argc, argv, n, filter_size, seed, verbose, debug);

	// Creation of the cqf (size is MB)
	Cqf cqf(filter_size, verbose);

	// uint64 generators
	default_random_engine generator;
	generator.seed(seed);
	uniform_int_distribution<uint64_t> distribution;

	// Debug
	unordered_set<uint64_t> verif;

	// Add the uints one by one into the cqf
	for (size_t i=0 ; i<n ; i++) {
		uint64_t val = distribution(generator);
		if (debug)
			verif.insert(val);
		cqf.insert(val);
	}

	// Verification
	if (debug) {
		// Query present
		uint64_t total_present = 0;
		cout << "Querying present values" << endl;
		for (const uint64_t val : verif) {
			uint64_t count = cqf.query(val);
			total_present += count;
		}
		cout << total_present << "/" << n << " are correctly present" << endl;

		// Query absent
		uint64_t total_absent = 0;
		cout << "Querying absent values" << endl;
		for (size_t i=0 ; i<n ; i++) {
			uint64_t val = distribution(generator);
			uint64_t count = cqf.query(val);
			total_absent += count == 0 ? 1 : 0;
		}
		cout << total_absent << "/" << n << " are correctly absent" << endl;
	}

	return 0;
}


void print_cmd() {
	cerr << "cmd line: random_uints -n <value> [-s <size>] [-r <rnd_seed>] [-v] [-d]" << endl;
	cerr << "   -n <val>: Number of random uint to insert." << endl;
	cerr << "   -s <val>: Filter size (in MB) [default: 4]." << endl;
	cerr << "   -r <val>: uint64_t random seed." << endl;
	cerr << "   -d: debug flag." << endl;
	cerr << "   -v: verbose flag." << endl;
}


void parse_cmd(int argc, char *argv[], size_t & n, size_t & filter_size, uint64_t & seed, bool & verbose, bool & debug) {
	int opt;

    while ((opt = getopt(argc, argv, "hn:s:vd")) != -1) {
        switch (opt) {
        	case 'h':
        		print_cmd();
        		exit(0);
            case 'n':
                n = atoi(optarg);
                break;
            case 'r': {
	    		istringstream iss(optarg);
                iss >> seed; }
                break;
            case 's': {
	    		istringstream iss(optarg);
                iss >> filter_size; }
                break;
            case 'v':
                verbose = true;
                break;
            case 'd':
                debug = true;
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
