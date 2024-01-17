
module load KMC/3.2.1

WORKING_DIR=/pasteur/appa/scratch/fandreac/cqf_tests
OUT_DIR=$WORKING_DIR/output
SAMPLE_DIR=$WORKING_DIR/samples
TEMP_DIR=$WORKING_DIR/temp

SAMPLE_1=$SAMPLE_DIR/AHX_ACXIOSF_6_1_C2FGHACXX.IND4_clean.fastq.gz
SAMPLE_2=$SAMPLE_DIR/AHX_AMHIOSF_3_1_C7T36ACXX.IND9_clean.fastq.gz 
SAMPLE_3=$SAMPLE_DIR/AHX_ATRIOSF_7_1_C0URMACXX.IND4_clean.fastq.gz

kmer_length=23
max_ram=24
min_count=1

for SAMPLE in $SAMPLE_1 $SAMPLE_2 $SAMPLE_3
do
    # KMC COUNTING AND DUMP INTO TEXTFILE
    kmc -k23 -m24 -ci1 -v $SAMPLE /scratch/vlevallois/data/6_1_23.res 
    kmc_tools transform dump 6_1_23.res AHX_ACXIOSF_6_1_23_all.txt

    # CQF

done