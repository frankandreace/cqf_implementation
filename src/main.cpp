#include "cqf.hpp"
#include <chrono>

std::chrono::steady_clock::time_point begin;


using namespace std;

 
int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << "Usage: ./bqf <command>" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "./cqf build -q <quotient size> [-k <k=32>] [-z <z=5>] -i <counted_smers> -o <BQF_file>" << std::endl;
        std::cout << "./cqf query -b <bqf_file> -i <reads_to_query> -o <results>" << std::endl;
        std::cout << "./cqf help" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    std::string input_file;
    std::string output_file;

    if (command == "build") {
        int q = 8;
        int k = 32;
        int z = 5;

        for (int i = 2; i < argc; i++) {
            if (std::string(argv[i]) == "-q") {
                if (i + 1 < argc) {
                    q = std::stoi(argv[i + 1]);
                } else {
                    std::cerr << "The -q option requires a value." << std::endl;
                    return 1;
                }
            } else if (std::string(argv[i]) == "-k") {
                if (i + 1 < argc) {
                    k = std::stoi(argv[i + 1]);
                } else {
                    std::cerr << "The -k option requires a value." << std::endl;
                    return 1;
                }
            } else if (std::string(argv[i]) == "-z") {
                if (i + 1 < argc) {
                    z = std::stoi(argv[i + 1]);
                } else {
                    std::cerr << "The -z option requires a value." << std::endl;
                    return 1;
                }
            } else if (std::string(argv[i]) == "-i") {
                if (i + 1 < argc) {
                    input_file = argv[i + 1];
                } else {
                    std::cerr << "The -i option requires an input file name." << std::endl;
                    return 1;
				}
        	} else if (std::string(argv[i]) == "-o") {
                if (i + 1 < argc) {
                    output_file = argv[i + 1];
                } else {
                    std::cerr << "The -o option requires an output file name." << std::endl;
                    return 1;
                }
            }
        }

        if (q <= 8) {
            std::cerr << "Values of 'q' must be greater than 8." << std::endl;
            return 1;
        }

        auto initialization_start = std::chrono::steady_clock::now();

		//Cqf cqf = Cqf(q,MEM_UNIT-q, k, z, false);
		//cqf.insert_fimpera(input_file);
        Cqf cqf = Cqf(q,MEM_UNIT-q, k, false);
        auto initialization_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - initialization_start).count();
        auto insertion_start = std::chrono::steady_clock::now();
		cqf.insert(input_file);
        auto insertion_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - insertion_start).count();
		auto saving_start = std::chrono::steady_clock::now();
        cqf.save_on_disk(output_file);
        auto saving_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - saving_start).count();
		
        std::cout << "CQF constructed successfully and saved to " << output_file << std::endl;
        std::cout << "Construction time = " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - initialization_start).count() << "ms" << std::endl;
        std::cout << "INITIALIZATION: " << initialization_time << "ms;  INSERTION: " << insertion_time << " ms; STORING TO DISK: " << saving_time << "ms." << endl;
    } 
	
	else if (command == "query") {

        std::string input_reads_file_to_query;
        for (int i = 2; i < argc; i++) {
            if (std::string(argv[i]) == "-b") {
                if (i + 1 < argc) {
                    input_file = argv[i + 1];
                } else {
                    std::cerr << "The -b option requires a value." << std::endl;
                    return 1;
                }
            } else if (std::string(argv[i]) == "-i") {
                if (i + 1 < argc) {
                    input_reads_file_to_query = argv[i + 1];
                } else {
                    std::cerr << "The -i option requires a value." << std::endl;
                    return 1;
                } 
            } else if (std::string(argv[i]) == "-o") {
                if (i + 1 < argc) {
                    output_file = argv[i + 1];
                } else {
                    std::cerr << "The -o option requires an output file name." << std::endl;
                    return 1;
                }
            } 
        }
        
        if (input_file.empty() || input_reads_file_to_query.empty()) {
            std::cerr << "Input file names are missing." << std::endl;
            std::cerr << "- input_file.empty()                = " << input_file.empty() << std::endl;
            std::cerr << "- input_file                        = " << input_file         << std::endl;
            std::cerr << "- input_reads_file_to_query.empty() = " << input_reads_file_to_query.empty() << std::endl;
            std::cerr << "- input_reads_file_to_query         = " << input_reads_file_to_query         << std::endl;
            std::cerr << "Error location : file = [" << __FILE__ << "] at line " << __LINE__ << std::endl;
            return EXIT_FAILURE;
        }

        uint64_t i = 0;
        auto begin = std::chrono::steady_clock::now();
		Cqf cqf = Cqf::load_from_disk(input_file);

		try {
			std::ifstream infile(input_reads_file_to_query);
            std::ofstream outfile(output_file);

			if (!infile) {
                std::cerr << "Exception happened..." << std::endl;
                std::cerr << "Error location : file = [" << __FILE__ << "] @ line " << __LINE__ << std::endl;
				throw std::runtime_error("File not found: " + input_reads_file_to_query);
			}
            if (!outfile) {
                std::cerr << "Exception happened..." << std::endl;
                std::cerr << "Error location : file = [" << __FILE__ << "] @ line " << __LINE__ << std::endl;
				throw std::runtime_error("File can not be created: " + output_file);
			}

			std::string read; 
			while (infile >> read) {
                i++;
                outfile << "Sequence" << i << " : " << cqf.query(read) << "\n";
                //std::cout << "query " << read << std::endl;
			}

			infile.close();
            outfile.close();
		} catch (const std::exception &e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}


        std::cout << "Queries executed successfully." << std::endl;
        std::cout << "Load + queries time = " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count() << "ms" << std::endl;
    
    } else if (command == "help" || command == "h") {
        std::cout << "Usage: ./bqf <command>" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "./bqf build -q <quotient size> [-c <count size=5>] [-k <k=32>] [-z <z=5>] -i <counted_smers> -o <BQF_file>" << std::endl;
        std::cout << "./bqf query -b <bqf_file> -i <reads_to_query>" << std::endl;
        std::cout << "./bqf help" << std::endl;

        std::cout << "-q is quotient size, it sets the filter size (there will be 2^q slots) so 2^(q-1) < nb_unique_elements < 2^q is needed" << std::endl;
        std::cout << "-c is the number of bits reserved for counters of each element. 2^c will be the maximum value" << std::endl;
        std::cout << "-k is the kmer size. The result of the query of a sequence S will be the minimum of the queries of all the kmers of S" << std::endl;
        std::cout << "-z is fimpera parameter. kmers are queried through the query of all their smers. s = k-z and smers are effectively inserted in the filter" << std::endl;
        std::cout << "-i is input_file, can be counted smers for \"build\" command or sequences to query for \"query\" command" << std::endl;
        std::cout << "-o is the file on which the BQF is saved in binary form after building (weights around 2^q*(3+c+r) bits, r being 2s-q)" << std::endl;
        std::cout << "-b is the file from which the BQF is loaded" << std::endl;
    } else {
        std::cerr << "Invalid command or incorrect number of arguments." << std::endl;
        return 1;
    }

    return 0;

}