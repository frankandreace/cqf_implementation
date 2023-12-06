#include "cqf.hpp"

using namespace std;

Cqf::Cqf() {}

Cqf::Cqf(uint64_t q_size, uint64_t r_size, bool verb)
{
    assert(q_size >= 7);

    verbose = verb;

    elements_inside = 0;
    quotient_size = q_size;

    remainder_size = r_size;

    if (verbose)
    {
        cout << "quotient_size: " << quotient_size << " remainder_size: " << remainder_size << endl;
    }

    uint64_t num_quots = 1ULL << quotient_size;
    uint64_t num_of_words = num_quots * (MET_UNIT + remainder_size) / MEM_UNIT;

    size_limit = num_quots * 0.95;

    // In machine words
    number_blocks = ceil(num_quots / BLOCK_SIZE);

    filter = vector<uint64_t>(num_of_words);
    if (verbose)
    {
        cout << "Construction done." << endl;
    }
}

Cqf::Cqf(uint64_t max_memory, bool verb)
{ // TO CHANGE, MISS HASH INFORMATION
    elements_inside = 0;

    verbose = verb;

    // Size of the quotient/remainder to fit into max_memory MB
    quotient_size = find_quotient_given_memory(max_memory);
    assert(quotient_size >= 7);
    remainder_size = MEM_UNIT - quotient_size;

    // Number of quotients must be >= MEM_UNIT
    uint64_t num_quots = 1ULL << quotient_size;                                 // 524.288
    uint64_t num_of_words = num_quots * (MET_UNIT + remainder_size) / MEM_UNIT; // 393.216

    // In machine words
    number_blocks = ceil(num_quots / BLOCK_SIZE);

    filter = vector<uint64_t>(num_of_words);
}

void Cqf::insert(string kmc_input)
{
    if (verbose)
    {
        cout << "kmc insertion" << endl;
    }
    try
    {
        ifstream infile(kmc_input);

        if (!infile)
        {
            throw std::runtime_error("File not found: " + kmc_input);
        }

        string kmer;
        uint64_t count;

        infile >> kmer >> count;
        if (kmer.size() == this->kmer_size)
        {
            this->insert(kmer, count);
        }
        else
        {
            std::cerr << "This CQF has been configured to accept " << this->kmer_size << "-mers but trying to insert " << kmer.size() << "-mers.\nEnd of insertions." << std::endl;
            return;
        }

        while (infile >> kmer >> count)
        {
            this->insert(kmer, count);
        }

        infile.close();
    }
    catch (const std::exception &e)
    {

        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void Cqf::insert(string kmer, uint64_t count)
{
    if (verbose)
    {
        cout << "k-mer insertion" << endl;
    }
    this->insert(kmer_to_hash(kmer, kmer_size), count);
}

void Cqf::insert(uint64_t number, uint64_t count)
{
    if (verbose)
    {
        cout << "insertion" << endl;
    }

    if (elements_inside + 1 == size_limit)
    {
        if (verbose)
        {
            cout << "RESIZING, nbElem: " << elements_inside << endl;
        }
        this->resize(1);
    }

    // get quotient q and remainder r
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);
    list<uint64_t> encoded_counter = encode_counter(rem, count);
    uint64_t slots_to_fill = encoded_counter.size();

    if (verbose)
    {
        cout << "[INSERT] quot " << quot << " from hash " << number << endl;
        cout << "[INSERT] rem " << rem << endl;
        cout << "[INSERT] counter ";
        for (auto const &i : encoded_counter)
        {
            std::cout << i << " ";
        }
        cout << endl;
    }

    // GET ALL THE UNUSED SLOTS NEEDED FOR THE COUNTER
    uint64_t fu_slot = first_unused_slot(quot);
    assert(get_remainder(fu_slot) == 0);

    list<uint64_t> free_slots;
    free_slots.push_back(fu_slot);
    while (--slots_to_fill > 0)
    {
        free_slots.push_back(first_unused_slot(get_next_quot(free_slots.back())));
    }

    if (verbose)
    {
        cout << "free slots list: ";
        print_list(free_slots);
    }

    if (!is_occupied(quot))
    {
        uint64_t starting_position = get_runstart(quot, 0);

        if (verbose)
        {
            cout << "SETTING OCCUPIED BIT "
                 << "Block id: " << get_block_id(quot) << "; shift in block: " << get_shift_in_block(quot) << endl;
        }

        set_occupied_bit(get_block_id(quot), 1, get_shift_in_block(quot));

        if (verbose)
        {
            cout << "INSERT COUNTER. starting_position: " << starting_position << "; counter = ";
            print_list(encoded_counter);
        }

        insert_counter_circ(encoded_counter, free_slots, starting_position);

        // METADATA HANDLING
        list<uint64_t> shift_metadata_slots = free_slots;
        shift_metadata_slots.push_front(get_prev_quot(starting_position));
        uint64_t shift_iterations = shift_metadata_slots.size() - 1;
        uint64_t start_range, end_range, flag;

        for (int i = 0; i < shift_iterations; i++)
        {
            end_range = shift_metadata_slots.back();
            shift_metadata_slots.pop_back();
            start_range = get_next_quot(shift_metadata_slots.back());
            if (i == shift_iterations - 1)
                flag = 1;
            else
                flag = 0;
            //if (start_range != end_range) 
            shift_bits_left_metadata(quot, flag, start_range, end_range, i + 1);
        }
        set_runend_bit(get_block_id(free_slots.back()),1,get_shift_in_block(free_slots.back()));
        elements_inside++;
        return;
    }
    // IF THE QUOTIENT HAS BEEN USED BEFORE
    // GET POSITION WHERE TO INSERT TO (BASED ON VALUE) IN THE RUN (INCREASING ORDER)
    else
    {
        if (verbose)
        {
            cout << "occupied" << endl;
        }
        // getting boundaries of the run
        pair<uint64_t, uint64_t> boundary = get_run_boundaries(quot);

        if (verbose)
        {
            cout << "boundaries " << boundary.first << " || " << boundary.second << endl;
        }

        // find the place where the remainder should be inserted / all similar to a query
        // getting position where to start shifting right
        counter_info count_position = scan_run(rem, boundary.first, boundary.second, greater_than); // uint64_t remainder, pair<uint64_t, uint64_t> range, F condition_met
        uint64_t starting_position = count_position.start_encoding;

        if (verbose)
        {
            cout << "starting_position " << starting_position << endl;
        }

        // set_occupied_bit(get_block_id(quot), 1, get_shift_in_block(quot));

        insert_counter_circ(encoded_counter, free_slots, starting_position);

        // METADATA HANDLING
        list<uint64_t> shift_metadata_slots = free_slots;
        shift_metadata_slots.push_front(starting_position - 1);
        uint64_t shift_iterations = shift_metadata_slots.size() - 1;
        uint64_t start_range, end_range, flag;
        if (verbose)
            {
                cout << "shift_iterations " << shift_iterations << endl;
            }

        for (int i = 0; i < shift_iterations; i++)
        {

            end_range = shift_metadata_slots.back();
            shift_metadata_slots.pop_back();
            start_range = get_next_quot(shift_metadata_slots.back());
            if (i == shift_iterations - 1)
                start_range = shift_metadata_slots.back();
            else
                start_range = get_next_quot(shift_metadata_slots.back());
            flag = 0;
            if (verbose)
            {
                cout <<"start_range " << start_range << "; end_range " << end_range << "; flag " << flag << endl;
            }

            shift_bits_left_metadata(quot, flag, start_range, end_range, i + 1);
        }

        elements_inside++;
        return;
    }
}

void Cqf::insert_counter_circ(list<uint64_t> counter, list<uint64_t> free_slots, uint64_t start_quotient)
{
    if (verbose)
    {
        cout << "[SET REM FROM LIST CIRC]: quotient " << start_quotient << endl;
    }
    assert(start_quotient < (1ULL << quotient_size));

    uint64_t slots_to_fill = counter.size();

    uint64_t curr_word_pos = get_remainder_word_position(start_quotient);
    uint64_t curr_word_shift = get_remainder_shift_position(start_quotient);

    uint64_t curr_free_slot = start_quotient;

    uint64_t end_quotient = get_next_quot(free_slots.back());
    while (start_quotient != end_quotient)
    {
        if (start_quotient == free_slots.front())
        {
            free_slots.pop_front();
        }
        else
        {
            counter.push_back(get_bits(filter, curr_word_pos * BLOCK_SIZE + curr_word_shift, remainder_size));
        }
        set_bits(filter, curr_word_pos * BLOCK_SIZE + curr_word_shift, counter.front(), remainder_size);
        counter.pop_front();
        if (curr_word_shift + remainder_size >= BLOCK_SIZE)
        {
            curr_word_shift = remainder_size - (BLOCK_SIZE - curr_word_shift); //(curr_word_shift + rem_size) % BLOCK_SIZE
            curr_word_pos = get_next_remainder_word(curr_word_pos);
        }
        else
        {
            curr_word_shift += remainder_size;
        }
        start_quotient = get_next_quot(start_quotient);
    }
}

void Cqf::shift_bits_left_metadata(uint64_t quotient, uint64_t overflow_bit, uint64_t start_position, uint64_t end_position, uint64_t shift_slots)
{
    if (verbose)
    {
        cout << "shift_bits_left_metadata quotient " << quotient << " SP " << start_position << " EP " << end_position << "N_BITS" << shift_slots << endl;
    }

    assert(start_position < (1ULL << quotient_size));
    assert(end_position < (1ULL << quotient_size));

    // METHOD FOR INSERTION
    uint64_t current_block = get_block_id(quotient);
    uint64_t current_shift_in_block = get_shift_in_block(start_position);

    uint64_t start_block = get_block_id(start_position);

    uint64_t end_block = get_block_id(end_position);
    uint64_t end_shift_in_block = get_shift_in_block(end_position);
    //(end_position == first unused slot)

    uint64_t word_to_shift;
    uint64_t save_right;
    uint64_t to_shift;
    uint64_t next_block;
    uint64_t save_left;

    uint64_t overflow_bits = shift_left(overflow_bit, shift_slots - 1);

    // OFFSET case quotient is in first slot (TEST_F(RsqfTest, offset2))
    if (get_shift_in_block(quotient) == 0)
    {
        set_offset_word(current_block, get_offset_word(current_block) + 1); // TO CHECK
    }

    // OFFSET case everyblock we cross until runstart
    while (current_block != start_block)
    {
        current_block = get_next_block_id(current_block);
        set_offset_word(current_block, get_offset_word(current_block) + 1); // TO CHECK
    }

    // current_block == start_block
    // RUNEND case runstart at the end of filter (shift almost all filter)
    if ((current_block == end_block) && (start_position > end_position))
    {
        word_to_shift = get_runend_word(current_block);
        save_right = word_to_shift & mask_right(current_shift_in_block);
        to_shift = shift_left(shift_right(word_to_shift, current_shift_in_block), (current_shift_in_block + shift_slots));

        overflow_bits = shift_left(overflow_bits, current_shift_in_block);
        to_shift |= (save_right | overflow_bits);
        set_runend_word(current_block, to_shift);
        overflow_bits = shift_right(word_to_shift, (MEM_UNIT - shift_slots));

        current_block = get_next_block_id(current_block);
        current_shift_in_block = 0;
        set_offset_word(current_block, get_offset_word(current_block) + shift_slots); // TO CHECK
    }

    while (current_block != end_block)
    {
        word_to_shift = get_runend_word(current_block);
        save_right = word_to_shift & mask_right(current_shift_in_block);
        to_shift = shift_left(shift_right(word_to_shift, current_shift_in_block), (current_shift_in_block + shift_slots));

        overflow_bits = shift_left(overflow_bits, current_shift_in_block);
        to_shift |= (save_right | overflow_bits);
        set_runend_word(current_block, to_shift);
        overflow_bits = shift_right(word_to_shift, (MEM_UNIT - shift_slots));

        next_block = get_next_block_id(current_block);
        current_block = next_block;

        // OFFSET case everyblock we cross until runend
        set_offset_word(current_block, get_offset_word(current_block) + shift_slots); // TO CHECK
        current_shift_in_block = 0;
    }

    word_to_shift = get_runend_word(current_block);
    save_left = (word_to_shift & mask_left(MEM_UNIT - end_shift_in_block));
    save_right = word_to_shift & mask_right(current_shift_in_block);

    to_shift = (((word_to_shift & mask_right(end_shift_in_block)) & mask_left(MEM_UNIT - current_shift_in_block)) << shift_slots);
    to_shift |= ((save_left | shift_left(overflow_bits, current_shift_in_block)) | save_right);

    set_runend_word(current_block, to_shift);
}

list<uint64_t> Cqf::encode_counter(uint64_t remainder, uint64_t count)
{
    assert(count > 0);
    uint64_t max_encodable_value = (1ULL << remainder_size) - 1;
    if (verbose)
    {
        cout << "Encoding remainder " << remainder << " with count " << count << endl;
        cout << "max_encodable_value " << max_encodable_value << " with remainder size " << remainder_size << endl;
    }
    // CASE 1: REMAINDER IS 0, USE GOOD STRATEGY TO ENCODE THE COUNTER
    // encode of 1 zero: 0
    // encode of > 1 zero: use '00' as flag (and counter of 1) and then use the same strategy as of any other remainder !=0.
    if (remainder == 0)
    {
        list<uint64_t> counter = {};

        counter.push_back(0);

        if (count > 1)
        {
            counter.push_back(0);
            // COUNT THE NUMBER OF SLOTS THAT ARE GOING TO BE FILLED BY THE MAXIMUM ENCODABLE VALUE
            uint64_t maximum_count_slots = (count - 2) / max_encodable_value;
            // GET THE VALUE OF THE LAST COUNTER SLOT - WHATERVER IS NOT ENCODED INTO THE MAXIMUM ENCODABLE VALUE SLOTS
            uint64_t count_last_slot = (count - 2) % max_encodable_value;
            // ADD THE MAXIMUM ENCODABLE VALUE SLOTS OF THE COUNTER
            while (maximum_count_slots-- > 0)
                counter.push_back(max_encodable_value);
            // ADD THE FINAL ENCODING, IF > 0
            if (count_last_slot > 0)
                counter.push_back(count_last_slot);
            // ADD THE ENDING REMAINDER AS CLOSING
            counter.push_back(0);
        }
        return counter;
    }
    // CASE 2: REMAINDER != 0, USE ROB PATRO'S STRATEGY
    else
    {
        // STARTING COUNTER ENCODING. FIRST THING IS THE REMAINDER VALUE
        list<uint64_t> counter = {remainder};
        if (count == 1){
            if (verbose)
            {
                print_list(counter);
            }
            return counter;
        }
        else if (count > 2){
            // COUNT THE NUMBER OF SLOTS THAT ARE GOING TO BE FILLED BY THE MAXIMUM ENCODABLE VALUE
            uint64_t maximum_count_slots = (count - 2) / (max_encodable_value - 1);
            // GET THE VALUE OF THE LAST COUNTER SLOT - WHATERVER IS NOT ENCODED INTO THE MAXIMUM ENCODABLE VALUE SLOTS
            uint64_t count_last_slot = (count - 2) % (max_encodable_value - 1);

            // IF THE COUNTER - 2 (2 COUNTS ARE IMPLICIT IN THE REMAINDERS AT THE BEINNING AND AT THE END)
            // IS GRATER THAN OR EQUAL TO THE REMAINDER VALUE, I NEED FOR SURE A ZERO AT THE SECOND POSITION

            if (verbose)
            {
                cout << "maximum_count_slots " << maximum_count_slots << "; count_last_slot " << count_last_slot << endl;
            }

            if ((count - 2) >= remainder)
                counter.push_back(0ULL);
            // ADD THE MAXIMUM ENCODABLE VALUE SLOTS OF THE COUNTER
            while (maximum_count_slots-- > 0)
            {
                counter.push_back(max_encodable_value);
            }
            // ADD THE FINAL ENCODING, IF > 0
            if (count_last_slot > 0)
            {
                if (count_last_slot > remainder) count_last_slot++;
                counter.push_back(count_last_slot);
            }
        }
        // ADD THE ENDING REMAINDER AS CLOSING
        counter.push_back(remainder);

        if (verbose)
        {
            print_list(counter);
        }
        return counter;
    }
}

counter_info Cqf::read_count(uint64_t value, uint64_t current_position, uint64_t end_position)
{                                                               // [interval)
    uint64_t count = 1;                                         // starting from the second element of the counter, so the first value has been read and the counter is 1.
    counter_info out_info;
    out_info.start_encoding = get_prev_quot(current_position);     // used to report the count and ending position to the calling function

    uint64_t curr_value;                                        // used to store the current value
    if (verbose) cout << "inside read count of " << value << " start: " << current_position << " end: " << end_position  << endl;
    // THIS PART IS DONE ONLY IN THE CASE IT IS EXPLORING THE COUNTER OF A 0. (IT IS A PARTICULAR CASE WITH A DIFFERENT ENCODING)
    if (value == 0)
    {
        curr_value = get_remainder(current_position); // reading the value in the position I am in
        if (curr_value != 0)
        {                        
            out_info.count = count;               // checking, if 0, it is encoding a 0 counter, if not, it is only 1 0
            out_info.end_encoding = current_position; // returing this position
            return out_info;                    // counter is not changed as there is only 1 zero.
        }
        current_position = get_next_quot(current_position); // if the value is 0, then it is encoding a 0 counter. reading the next value
    }

    // THIS PART WORKS FOR ANY VALUE != 0 AND FOR THE "SECOND" PART OF THE 0 COUNTER
    while (current_position != get_next_quot(end_position))
    {                                                 // used to not go out of run bounds.
        if (verbose) cout << current_position << endl;
        curr_value = get_remainder(current_position); // reading the value
        if (verbose) cout << "reading count" << curr_value << endl;
        if (curr_value == value)
        {                                                      // IN CASE I READ AGAIN THE REMAINDER VALUE, THIS IS AN EXIT FLAG FOR THE COUNTER. THE TOTAL COUNT IS GOING TO BE RETURNED
            out_info.count = ++count;                          // INCREASE THE COUNT BY ONE AND STORE IT
            out_info.end_encoding = current_position; // GIVE THE NEXT POSITION, I.E. THE ONE WHERE TO START FOR THE NEXT SEARCH
            if (verbose) cout << "returning " << value << " " << out_info.count << " " << out_info.start_encoding << " " << out_info.end_encoding << endl;
            return out_info;
        }
        // IF CURRENT VALUE != REMAINDER VALUE, IT IS A COUNTER ENCODED
        else if (curr_value > value)
        {
            count += curr_value - 1; // IF THE COUNTER VALUE > REMAINDER VALUE IT ENCODES FOR CURR_VALUE - 1 (REMAINDER VALUE CANNOT BE USED TO ENCODE FOR A COUNTER VALUE)
        }
        else
        {
            count += curr_value; // IF COUNTER VALUE < REMAINDER VALUE, IT ENCODES FOR CURR_VALUE
        }
        current_position = get_next_quot(current_position); // ADJOURNING THE POSITION
    }
    throw std::runtime_error("Error while decoding the count of " + value); // IF I EXIT THE LOOP WITHOUT DECODING THE GIVEN COUNTER, THERE IS A PROBLEM
}

std::map<uint64_t, uint64_t> Cqf::enumerate()
{
    std::map<uint64_t, uint64_t> finalSet;
    std::vector<std::pair<uint64_t, uint64_t>> curr_run_elements;
    uint64_t probe;

    std::pair<uint64_t, uint64_t> bounds;
    uint64_t current_position;

    uint64_t quotient;
    uint64_t current_value;

    if (verbose)
    {
        cout << "enumerating. "<< ceil(number_blocks/10) << endl;
    }
    for (int64_t block = 0; block < number_blocks; ++block)
    {
        probe = get_occupied_word(block);
        if (verbose)
        {
            cout << "At block " << block << " the occupied word is " << probe << endl;
        }
        if (probe == 0)
            continue;
        for (uint64_t i = 0; i < BLOCK_SIZE; i++)
        {
            if (probe & 1ULL)
            { // occupied
                quotient = block * BLOCK_SIZE + i;
                bounds = get_run_boundaries(quotient);
                if (verbose) cout << "FOUND AN OCCUPIED BIT IN (quotient) " << quotient << "; with bounds " << bounds.first << " - " << bounds.second << endl;
                curr_run_elements = report_run(bounds.first, bounds.second);
                for (int i = 0; i < curr_run_elements.size(); i++)
                {
                    if (verbose) cout << "inserting in verification map remainder " << curr_run_elements[i].first << " with count " << curr_run_elements[i].second << endl;
                    finalSet[rebuild_number(quotient, curr_run_elements[i].first, quotient_size)] = curr_run_elements[i].second;
                }
            }
            probe >>= 1ULL; // next bit of occupied vector
        }
    }
    return finalSet;
}

void Cqf::display_vector(){
    uint64_t block_pos;
    for (int64_t block = 0; block < number_blocks; ++block)
    {
        block_pos = block * BLOCK_SIZE;
        cout << get_offset_word(block) << endl;
        print_bits(get_occupied_word(block));
        print_bits(get_runend_word(block));
        for (uint64_t i = 0; i < BLOCK_SIZE; i++){
            cout << get_remainder(block_pos + i) << " ";
        }
        cout << endl << endl;
    }
}


std::vector<std::pair<uint64_t, uint64_t>> Cqf::report_run(uint64_t current_position, uint64_t end_position)
{
    uint64_t count = 0;
    uint64_t max_encodable_value = (1ULL << remainder_size) - 1;
    uint64_t old_value;
    uint64_t curr_value = get_remainder(current_position);
    counter_info decoded_info;
    std::pair<uint64_t, uint64_t> report_info;
    std::vector<std::pair<uint64_t, uint64_t>> report;

    // CASE 1: 1st REMAINDER IS 0
    if (current_position == end_position) {
        report_info.first = curr_value;
        report_info.second = 1;
        report.push_back(report_info);
        return report;
    }

    if (curr_value == 0)
    {
        decoded_info = read_count(curr_value, current_position, end_position);
        report_info.first = curr_value;
        report_info.second = decoded_info.count;
        report.push_back(report_info);
        current_position = get_next_quot(decoded_info.end_encoding);
        curr_value = get_remainder(current_position);
    }

    while (current_position != end_position)
    {
        if (verbose) cout << current_position << endl;
        old_value = curr_value;
        current_position = get_next_quot(current_position);
        curr_value = get_remainder(current_position);

        if (curr_value < old_value)
        { // A COUNTER IS ENCODED
            if (verbose){
                cout << "read count of " << old_value << "; from " << current_position << " to " << end_position << endl; 
            }
            decoded_info = read_count(old_value, current_position, end_position);
            report_info.first = old_value;
            report_info.second = decoded_info.count;
            report.push_back(report_info);
            current_position = decoded_info.end_encoding;
            curr_value = get_remainder(current_position);
        }
        else if (curr_value == old_value)
        {
            report_info.first = old_value;
            report_info.second = 2;
            report.push_back(report_info);
        }
    }
    return report;
}