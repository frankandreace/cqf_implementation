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
    // get quotient q and remainder r
    if (count == 0)
    {
        return;
    }
    number &= (mask_right(this->quotient_size + this->remainder_size));
    uint64_t quot = quotient(number);
    uint64_t rem = remainder(number);
    list<uint64_t> encoded_counter = encode_counter(rem, count);
    uint64_t slots_to_fill = encoded_counter.size();
    uint64_t slots_to_add_count_inserted = slots_to_fill;

    if (verbose)
    {
        cout << "insertion" << endl;
    }

    if (elements_inside + slots_to_fill == size_limit)
    {
        if (verbose)
        {
            cout << "RESIZING, # Slots used: " << elements_inside << " (" << elements_inside/double(number_blocks*MEM_UNIT) << ") with " << number_blocks*MEM_UNIT << " available slots, num integers inserted: " << num_uint_inserted << endl;
        }
        this->resize(1);
        quot = quotient(number);
        rem = remainder(number);
        encoded_counter = encode_counter(rem, count);
        slots_to_fill = encoded_counter.size();
        slots_to_add_count_inserted = slots_to_fill;
        if (verbose)
        {
            cout << "!! FINISHED RESIZING, # Slots used: " << elements_inside << " (" << elements_inside/double(number_blocks*MEM_UNIT) << ") with " << number_blocks*MEM_UNIT << " available slots, num integers inserted: " << num_uint_inserted << endl;
            cout << "Now quotient is " << this->quotient_size << " and remainder is " << this->remainder_size << endl;
        }
    }

    if (verbose)
    {
        cout << "[INSERT] quot " << quot << " from hash " << number << endl;
        cout << "[INSERT] rem " << rem << endl;
        cout << "[INSERT] slots_to_fill " << slots_to_fill << endl;
        cout << "[INSERT] count = " << count << " ;counter ";
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
        if (verbose)
        {
            cout << "SETTING OCCUPIED BIT "
                 << "Block id: " << get_block_id(quot) << "; shift in block: " << get_shift_in_block(quot) << endl;
        }

        set_occupied_bit(get_block_id(quot), 1, get_shift_in_block(quot));

        uint64_t starting_position = get_runstart(quot, 0);
        if (verbose)
        {
            cout << "INSERT COUNTER. starting_position: " << starting_position << "; counter = ";
            print_list(encoded_counter);
        }

        insert_counter_circ(encoded_counter, free_slots, starting_position);


        // METADATA HANDLING
        list<uint64_t> shift_metadata_slots = free_slots;
        shift_metadata_slots.push_front(starting_position);
        uint64_t shift_iterations = shift_metadata_slots.size() - 1;
        uint64_t start_range, end_range;
        uint64_t flag = 0;
        for (int i = 0; i < shift_iterations; i++)
        {
            end_range = shift_metadata_slots.back();
            shift_metadata_slots.pop_back();
            start_range = shift_metadata_slots.back();
            shift_bits_left_metadata(quot, flag, start_range, end_range, i + 1);
        }
        if (verbose) { cout << "Calculating runend bit position at " ;}
        uint64_t runend_bit_position = starting_position;
        if (slots_to_add_count_inserted > 1){ // REMOVE REDUNDANT IF
            for(int64_t i = 0; i < slots_to_add_count_inserted -1 ; i++){
                runend_bit_position = get_next_quot(runend_bit_position);
            }
        }
        if (verbose) { cout << "Setting runend bit at blockid " << get_block_id(runend_bit_position) << " and shift " << get_shift_in_block(runend_bit_position) << endl;}
        set_runend_bit(get_block_id(runend_bit_position), 1, get_shift_in_block(runend_bit_position));
        if (verbose) { cout << "Adding elements inside as " << elements_inside << " + " << slots_to_add_count_inserted << endl;}
        elements_inside = elements_inside + slots_to_add_count_inserted;
        if (verbose) { cout << "Adding integers inserted to " << (num_uint_inserted + 1) << endl;}
        num_uint_inserted++;
        if (verbose) { cout << "Returning" << endl;}
        return;
    }
    // IF THE QUOTIENT HAS BEEN USED BEFORE
    // GET POSITION WHERE TO INSERT TO (BASED ON VALUE) IN THE RUN (INCREASING ORDER)
    else
    {
        if (verbose){
            cout << "occupied" << endl;
        }
        // getting boundaries of the run
        pair<uint64_t, uint64_t> boundary = get_run_boundaries(quot);

        if (verbose)
            cout << "boundaries " << boundary.first << " || " << boundary.second << endl;

        // find the place where the remainder should be inserted / all similar to a query
        // getting position where to start shifting right
        counter_info run_information = scan_run(rem, boundary.first, boundary.second); // uint64_t remainder, pair<uint64_t, uint64_t> range, F condition_met
        if (verbose)
        {
            cout << "counter_position returned value " << run_information.value << "count: " << run_information.count << " ; start: " << run_information.position << " ;" << endl;
        }

        uint64_t starting_position = run_information.position;

        if (verbose)
        {
            cout << "starting_position " << starting_position << endl;
        }

        // set_occupied_bit(get_block_id(quot), 1, get_shift_in_block(quot));
        insert_counter_circ(encoded_counter, free_slots, starting_position);

        // METADATA HANDLING
        list<uint64_t> shift_metadata_slots = free_slots;
        if (starting_position == get_next_quot(boundary.second))
        {
            shift_metadata_slots.push_front(boundary.second);
        }
        else
        {
            shift_metadata_slots.push_front(starting_position);
        }
        
        uint64_t shift_iterations = shift_metadata_slots.size() - 1;
        uint64_t start_range, end_range;
        uint64_t flag = 0;
        if (verbose)
        {
            cout << "shift_iterations " << shift_iterations << endl;
        }

        for (int i = 0; i < shift_iterations; i++)
        {
            end_range = shift_metadata_slots.back();
            shift_metadata_slots.pop_back();
            start_range = shift_metadata_slots.back();
            if (verbose)
            {
                cout << "start_range " << start_range << "; end_range " << end_range << "; flag " << flag << endl;
            }
            shift_bits_left_metadata(quot, flag, start_range, end_range, i + 1);
        }

        elements_inside = elements_inside + slots_to_add_count_inserted;
        num_uint_inserted++;
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
        cout << "shift_bits_left_metadata quotient " << quotient << " SP " << start_position << " EP " << end_position << " N_BITS " << shift_slots << endl;
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
        if (verbose) {cout << "!!! SETTING OFFSET OF " << current_block << " TO " << get_offset_word(current_block) << endl;}
    }

    // OFFSET case everyblock we cross until runstart
    uint64_t offset_while = 0;
    while (current_block != start_block)
    {
        current_block = get_next_block_id(current_block);
        set_offset_word(current_block, get_offset_word(current_block) + 1); // TO CHECK
        cout << "!!! INCREASING OFFSET OF " << current_block << " TO " << get_offset_word(current_block) << endl;
        offset_while++;
    }
    if (verbose)
    {
        cout << "Offset increased " << offset_while << " times. Now at" << get_offset_word(current_block) << endl;
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
    if (verbose) { cout << "exiting shift" << endl;}
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
        if (count == 1)
        {
            if (verbose)
            {
                print_list(counter);
            }
            return counter;
        }
        else if (count > 2)
        {
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
                if (count_last_slot >= remainder)
                    count_last_slot++;
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

counter_info Cqf::scan_run(uint64_t remainder, uint64_t current_position, uint64_t end_position)
{
    uint64_t count = 0;
    uint64_t current_value = get_remainder(current_position);
    uint64_t next_position;
    counter_info info;
    info.value = current_value;
    info.count = count;
    info.position = current_position;
    end_position = get_next_quot(end_position);

    if (current_value == 0)
    {
        next_position = get_next_quot(current_position);
        if ((next_position != end_position) && (get_remainder(next_position) == 0))
        {
            tie(count, current_position) = read_count(current_value, get_next_quot(next_position), end_position);
            //count++;
        }
        else
        {
            count = 1;
            //current_position = get_next_quot(current_position);
        }
        if (current_value >= remainder)
        {
            info.value = current_value;
            info.count = count;
            info.position = current_position;
            return info;
        }

        current_position = get_next_quot(current_position);
        current_value = get_remainder(current_position);        
    }

    while (current_position != end_position)
    {
        if (current_value > remainder)
        {
            info.value = current_value;
            info.count = count;
            info.position = current_position;
            if (verbose)
            {
                cout << "[SCAN_RUN] Return in loop '>' " << current_position << endl;
            }
            return info;
        }
        next_position = get_next_quot(current_position);
        
        if (verbose)
        {
            cout << "[SCAN_RUN] Loop in position " << current_position << " with value " << current_value << " and remainder " << remainder << endl;
        }

        if ((next_position != end_position) && (get_remainder(next_position) <= current_value))
        {
            tie(count, current_position) = read_count(current_value, next_position, end_position);
            if (verbose)
            {
                cout << "[SCAN_RUN] Read count returned count " << count << " and position " << current_position << endl;
            }
        }
        else
        {
            count = 1;
        }
        if (current_value == remainder)
        {
            info.value = current_value;
            info.count = count;
            info.position = current_position;
            if (verbose)
            {
                cout << "Return in loop '==' value: " << info.value << " ;count " << info.count << " ;position " << info.position << endl;
            }
            return info;
        }
        current_position = get_next_quot(current_position);
        current_value = get_remainder(current_position);

    }
    info.count = 0;
    info.value = current_value;
    info.position = current_position;
    if (verbose)
    {
        cout << "Return out loop " << current_position << endl;
    }
    return info;
}

pair<uint64_t, uint64_t> Cqf::read_count(uint64_t value, uint64_t current_position, uint64_t end_position)
{
    uint64_t count = 2;
    uint64_t current_value;
    while (current_position != end_position)
    {
        current_value = get_remainder(current_position);
        if (current_value == value)
        {
            return {count, current_position};
        }
        if ((current_value < value) || (value == 0))
        {
            count = count + current_value;
        }
        else
        {
            count = count + current_value - 1;
        }
        current_position = get_next_quot(current_position);
    }
    throw std::runtime_error("Error while decoding the count of " + value);
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
        cout << "enumerating. " << ceil(number_blocks / 10) << endl;
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
                if (verbose)
                    cout << "FOUND AN OCCUPIED BIT IN (quotient) " << quotient << "; with bounds " << bounds.first << " - " << bounds.second << endl;
                curr_run_elements = report_run(bounds.first, bounds.second);
                for (int i = 0; i < curr_run_elements.size(); i++)
                {
                    if (verbose)
                        cout << "!!! INSERING in verification map value " << rebuild_number(quotient, curr_run_elements[i].first, quotient_size) << " with quotient " << quotient << " ; remainder " << curr_run_elements[i].first << " and count " << curr_run_elements[i].second << endl;
                    finalSet[rebuild_number(quotient, curr_run_elements[i].first, quotient_size)] = curr_run_elements[i].second;
                }
            }
            probe >>= 1ULL; // next bit of occupied vector
        }
    }
    if (verbose)
    {
        cout << "Finished enumerating. I had " << num_uint_inserted << " integers inside and I returned " << finalSet.size() << " integers." << endl;
    }
    return finalSet;
}

void Cqf::display_vector()
{
    uint64_t block_pos;
    for (int64_t block = 0; block < number_blocks; ++block)
    {
        block_pos = block * BLOCK_SIZE;
        cout << get_offset_word(block) << endl;
        print_bits(get_occupied_word(block));
        print_bits(get_runend_word(block));
        for (uint64_t i = 0; i < BLOCK_SIZE; i++)
        {
            cout << get_remainder(block_pos + i) << " ";
        }
        cout << endl
             << endl;
    }
}

std::vector<std::pair<uint64_t, uint64_t>> Cqf::report_run(uint64_t current_position, uint64_t end_position)
{
    //uint64_t count = 0;
    //uint64_t max_encodable_value = (1ULL << remainder_size) - 1;
    uint64_t current_value = get_remainder(current_position);
    counter_info decoded_info;
    std::pair<uint64_t, uint64_t> report_info;
    std::vector<std::pair<uint64_t, uint64_t>> report;
    while (current_position != get_next_quot(end_position))
    {
        if (verbose)
            cout << "REPORT_RUN. Position: " << current_position << endl;
        decoded_info = scan_run(current_value, current_position, end_position);
        report_info.first = current_value;
        report_info.second = decoded_info.count;
        report.push_back(report_info);
        current_position = get_next_quot(decoded_info.position);
        current_value = get_remainder(current_position);
    }

    // CASE CURR_POS == END_POS
    /*if (verbose)
        cout << "REPORT_RUN.END. Position: " << current_position << endl;
    report_info.first = current_value;
    report_info.second = 1;
    report.push_back(report_info);
    */
    return report;
}


void Cqf::resize(int n){
    std::map<uint64_t, uint64_t> inserted_elements = this->enumerate();

    this->quotient_size += n;
    this->remainder_size -= n;
    
    uint64_t num_quots = 1ULL << this->quotient_size; 
    uint64_t num_of_words = num_quots * (MET_UNIT + this->remainder_size) / MEM_UNIT;

    this->size_limit = num_quots * 0.95;

    // In machine words
    this->number_blocks = std::ceil(num_quots / BLOCK_SIZE);

    this->filter = std::vector<uint64_t>(num_of_words);
    this->elements_inside = 0;
    this->num_uint_inserted = 0;

    for (auto const& elem : inserted_elements){
        this->insert(elem.first, elem.second);
    }
}