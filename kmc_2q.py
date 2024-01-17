from sys import argv
with open(argv[1]) as f:
    for line in f:
        print(line.strip().split('\t')[0])