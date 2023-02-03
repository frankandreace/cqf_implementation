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
	if (verbose) cout << "insertions 0/" << n << "\n";
	for (size_t i=0 ; i<n ; i++) {
		uint64_t val = distribution(generator);
		uint64_t q = val & ((1ULL << 19) - 1);
		
		//if((val == 14541508102286271226ULL) or (val == 16770102985076564731ULL)){
		if((q == 320251) or (q == 320250)){
			uint64_t q = val & ((1ULL << 19) - 1);
			uint64_t rem = cqf.remainder(val);
			std::cout << "[DB]: quotient " << q << std::endl;
			std::cout << "[DB]: rem " << rem  << std::endl;
			std::cout << "[DB]: value " << val << std::endl;
			std::cout << "[DB]: block " << q/64 << std::endl;
			std::cout << "[DB]: pos in block " << q%64 << std::endl;

			std::cout << "[DB]: block" << (q/64) << " (the one of the quotient)" <<  std::endl;
			cout << endl << cqf.block2string(q/64, false) << endl << endl;
			}
		/*
		if (q == 53823) {
			cout << endl << cqf.block2string(840, false) << endl << endl;
			cout << endl << cqf.block2string(841, false) << endl << endl;
		}
		*/
		if (debug)
			verif.insert(val);
		cqf.insert(val);

		//if((val == 14541508102286271226ULL) or (val == 16770102985076564731ULL)){
		if((q == 320250) or (q == 320251)){
			uint64_t q = val & ((1ULL << 19) - 1);
			uint64_t rem = cqf.remainder(val);
			std::cout << "[DB]: quotient " << q << std::endl;
			std::cout << "[DB]: rem " << rem  << std::endl;
			std::cout << "[DB]: value " << val << std::endl;
			std::cout << "[DB]: block " << q/64 << std::endl;
			std::cout << "[DB]: pos in block " << q%64 << std::endl;

			std::cout << "[DB]: block" << (q/64) << " (the one of the quotient)" <<  std::endl;
			cout << endl << cqf.block2string(q/64, false) << endl << endl;
			}
		/*
		if (q == 53823) {
			cout << endl << cqf.block2string(840, false) << endl << endl;
			cout << endl << cqf.block2string(841, false) << endl << endl;
		}
		*/
		if (verbose){
			for (const uint64_t check : verif){
				if (cqf.query(check) != 1) cout << "PROBLEM WITH " << check << endl;
			}
			cout << "insertions " << (i+1) << "/" << n << "\n";

		}
	} cout << endl;

	// cout << endl << cqf.block2string(839, false) << endl << endl;
	// cout << endl << cqf.block2string(840, false) << endl << endl;
	// cout << endl << cqf.block2string(841, false) << endl << endl;
	// exit(0);

	// Verification
	if (debug) {
		// Query present
		uint64_t total_present = 0;
		cout << "Querying present values" << endl;
		for (const uint64_t val : verif) {
			cout << "QUERYING " << val << endl;
			uint64_t count = cqf.query(val);
			total_present += count;
			if (count != 1) cerr << "Missing value " << val << endl;
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

	uint64_t el = 16770102985076564731ULL;
	uint64_t q = el & ((1ULL << 19) - 1);
	uint64_t rem = cqf.remainder(el);
	
	std::cout << "[DB]: quotient " << q << std::endl;
	std::cout << "[DB]: rem " << rem  << std::endl;
	std::cout << "[DB]: value " << el << std::endl;
	std::cout << "[DB]: block " << q/64 << std::endl;
	std::cout << "[DB]: pos in block " << q%64 << std::endl;

	std::cout << "[DB]: block" << (q/64)-1 << " (before)" <<  std::endl;
	cout << endl << cqf.block2string((q/64)-1, false) << endl << endl;
	std::cout << "[DB]: block" << (q/64) << " (the one of the quotient)" <<  std::endl;
	cout << endl << cqf.block2string(q/64, false) << endl << endl;
	std::cout << "[DB]: block" << (q/64)+1 << " (after)" << std::endl;
	cout << endl << cqf.block2string((q/64)+1, false) << endl << endl;

	el = 14541508102286271226ULL;
	q = el & ((1ULL << 19) - 1);
	rem = cqf.remainder(el);
	
	std::cout << "[DB]: quotient " << q << std::endl;
	std::cout << "[DB]: rem " << rem  << std::endl;
	std::cout << "[DB]: value " << el << std::endl;
	std::cout << "[DB]: block " << q/64 << std::endl;
	std::cout << "[DB]: pos in block " << q%64 << std::endl;

	std::cout << "[DB]: block" << (q/64)-1 << " (before)" <<  std::endl;
	cout << endl << cqf.block2string((q/64)-1, false) << endl << endl;
	std::cout << "[DB]: block" << (q/64) << " (the one of the quotient)" <<  std::endl;
	cout << endl << cqf.block2string(q/64, false) << endl << endl;
	std::cout << "[DB]: block" << (q/64)+1 << " (after)" << std::endl;
	cout << endl << cqf.block2string((q/64)+1, false) << endl << endl;

	cout << "" << cqf.query(16770102985076564731ULL) << endl;
	cout << "" << cqf.query(14541508102286271226ULL) << endl;
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

    while ((opt = getopt(argc, argv, "hn:s:r:vd")) != -1) {
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
