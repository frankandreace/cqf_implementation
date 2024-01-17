import random
import time

def gen_kmer(k):
    return ''.join(random.choice('ACGT') for _ in range(k)) + '\n'

start_time = time.time()

output_file_path = "neg_queries.fasta"

with open(output_file_path, 'w') as output_file:
    for _ in range(100000000):
        sequence = gen_kmer(random.randint(80,120))
        output_file.write(sequence)

print("--- %s seconds ---" % (time.time() - start_time))