#ifndef CQF_HPP
#define CQF_HPP

#include <map>
#include <list>
#include <algorithm>
#include <limits>
#include <string>

#include "rsqf.hpp"

class Cqf : public Rsqf
{
public:
    uint64_t smer_size;
    uint64_t kmer_size;

    Cqf();

    // CQF WITHOUT FIMPERA
    Cqf(uint64_t q_size, uint64_t r_size, uint64_t k, bool verb);

    // CQF WITH FIMPERA
    Cqf(uint64_t q_size, uint64_t c_size, uint64_t k, uint64_t z, bool verb);

    Cqf(uint64_t max_memory, bool verb);

    /**
     * \brief query a sequence from the filter WITHOUT FIMPERA.
     *
     * Every abundance of each kmer of the query sequence will be queried. This is done following Fimpera scheme
     * and the smallest count amongst them will be returned.
     *
     * \param seq the sequence to query
     * \return the abundance of the given kmer in the filter
     */
    result_query query(std::string seq);

    /**
     * \brief query a sequence from the filter WITH FIMPERA.
    result_query query_fimpera(string seq);
     */
    result_query query_fimpera(std::string seq);

    /**
     * \brief Insert every kmer + abundance of a kmer count software output file (eg KMC)  WITHOUT FIMPERA.
     *
     * This function will read every line of the file to insert every pair <kmer, count> into the CQF
     *
     * \param file path to file
     */
    void insert(std::string file);

    /**
     * \brief Insert every kmer + abundance of a kmer count software output file (eg KMC)  WITH FIMPERA.
     *
     * This function will read every line of the file to insert every pair <kmer, count> into the CQF
     *
     * \param file path to file
     */
    void insert_fimpera(std::string kmc_input);

    void save_on_disk(const std::string& filename);

    static Cqf load_from_disk(const std::string& filename);

    void get_num_inserted_elements(){
        std::cout << "This filter contains " << this->num_uint_inserted << " filling " << this->elements_inside << " slots." << std::endl;
    }

private:

    uint64_t num_uint_inserted = 0;

    /**
     * \brief Set the remainder slot to the assigned value
     *
     * This function inserts a number in the CQF.
     * If a number has the same quotient but different remainders, it stores remainders in a monothonic way
     * (i.e. each remainder in a run is greater or equal than the predecessor).
     * If the filter is full, new insertions are authomatically discarded.
     *
     * \param counter the list of integers that encode the counter for the remainder
     * \param free_slots list of position where there are free slots - it is used in the operation of inserting the counter in the remainder slots
     * \param starting_quotient the position of the first remainder to move
     */
    void insert_counter_circ(std::list<uint64_t>& counter, std::list<uint64_t> free_slots, uint64_t starting_quotient, uint64_t quotient);

    /**
     * \brief Metadata function that shifts the bits left of X bits in the runend word when there is a counter insertion.
     * It also handles cases when occupieds and offsets have to be adjourned.
     * \param quotient quotient of the number to add.
     * \param flag_bit flags (with 1) if the occupied bit has to be set to 1, else it is 0.
     * \param start_position starting position of the shifting.
     * \param end_position end position of the shifting.
     * \param n_bits number of bits to shift
     */
    void shift_bits_left_metadata(uint64_t start_position, uint64_t end_position, uint64_t n_bits); //uint64_t quotient, uint64_t overflow_bit,
    

    /**
     * \brief query a number from the filter.
     *
     * It queries a number. It first checks if the occupied bit of the quotient is set to 1. If so it scans
     * in a linear way the remainders of the run associated to this element. If it
     * finds the remainder it returns its exact count (or order of magnitude, depending of the filter used ) else 0.
     * Stops immediately if the filter is empty
     *
     * \param number Number to query
     * \return the abundance of the given number in the filter
     */
    uint64_t query(uint64_t number);

        /**
     * TODO: allow user to choose hash function
     * \brief Insert a kmer in the filter alongside with his count.
     *
     * This function inserts a kmer in the CQF.
     * It is advised that the kmer is in a canonical form, it will be hashed then inserted.
     *
     * \param kmer to insert
     * \param count kmer abundance
     */
    void insert(std::string kmer, uint64_t count);

    /**
     * \brief Insert a NEW number in the filter alongside with his count (reduced to the power of 2 just below).
     *
     * This function inserts a number in the CQF.
     * If a number has the same quotient but different remainders, it stores remainders in a monothonic way
     * (i.e. each remainder in a run is greater or equal than the predecessor).
     * If the filter is full, new insertions are authomatically discarded.
     *
     * \param number to insert
     * \param count number of occurences of the element to insert (default: 1)
     */
    void insert(uint64_t number, uint64_t count = 1);

    /**
     * \brief encode a counter for a remainder in the filter.
     *
     * It encodes a counter. It first checks if the remainder is 0, as the encoding strategy is different.
     *
     * \param remainder the remainder value, as 0 counters are diff than others
     * \param count the count you want to encode for the remainder
     * \return a linked list containing the encoded counter for the remainder
     */

    std::list<uint64_t> encode_counter(uint64_t remainder, uint64_t count);

    /**
     * \brief looks for the counter of the remainder in the given range. If not found returns 0
     *
     * \param remainder the remainder value
     * \param range the start and end quotient of the run
     * \return a uint64_t containing the value of the counter, or 0 if not present
     */
    //template <typename F>
    counter_info scan_run(uint64_t remainder, uint64_t current_position, uint64_t end_position);

    std::map<uint64_t, uint64_t> enumerate();

    std::pair<uint64_t,uint64_t> read_count(uint64_t value, uint64_t current_position, uint64_t end_position);

    std::vector<std::pair<uint64_t, uint64_t>> report_run(uint64_t current_position, uint64_t end_position);

    void resize(int n);

    void display_vector();

    void compare_with_map(std::map<uint64_t,uint64_t> check);
};

#endif