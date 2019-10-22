from timeit import default_timer as timer
import subprocess
import numpy as np
import sys

'''
usage: python performance.py <luthien_location> <fastp_location>
'''

fastp = sys.argv[2]
sickle = sys.argv[1]

big_single = "big/SRR941557.fastq"
big_single_out = "output/SRR941557.fastq"
big_pair_1 = "big/SRR3309317_1.mini.fastq"
big_pair_2 = "big/SRR3309317_2.mini.fastq"
big_pair_1_out = "output/SRR3309317_1.mini.fastq"
big_pair_2_out = "output/SRR3309317_2.mini.fastq"

small_single = "test.fastq"
small_single_out = "output/test.fastq"
small_pair_1 = "test.f.fastq"
small_pair_2 = "test.r.fastq"
small_pair_1_out = "output/test.f.fastq"
small_pair_2_out = "output/test.r.fastq"

n_tests = 10

def runCommand(cmd):
    #print("\t> " + cmd)
    process = subprocess.call(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    #process = subprocess.call(cmd, shell=True)
    return process

def run_luthien(threads, chunk_size):
    #print("Running test for " + str(threads) + " " + str(chunk_size))
    duration_single = 0.0
    duration_paired = 0.0

    code = runCommand(" ".join(["../luthien -i1", big_pair_1, "-o1", big_pair_1_out,
                    "-i2", big_pair_2, "-o2", big_pair_2_out,
                    "-t", str(threads), "-c", str(chunk_size)]))
    for i in range(n_tests):
        start = timer()
        code = runCommand(" ".join(["../luthien -i1", big_pair_1, "-o1", big_pair_1_out,
                    "-i2", big_pair_2, "-o2", big_pair_2_out,
                    "-t", str(threads), "-c", str(chunk_size)]))
        if(code != 0):
            quit()
        end = timer()
        duration_paired += (end-start)
    
    code = runCommand(" ".join(["../luthien -i1", big_single, "-o1", big_single_out, 
                    "-t", str(threads), "-c", str(chunk_size)]))
    for i in range(n_tests):
        start = timer()
        code = runCommand(" ".join(["../luthien -i1", big_single, "-o1", big_single_out, 
                    "-t", str(threads), "-c", str(chunk_size)]))
        end = timer()
        if(code != 0):
            quit()
        duration_single += (end-start)
    
    return duration_single / n_tests, duration_paired / n_tests

def run_fastp(threads):
    duration_single = 0.0
    duration_paired = 0.0

    cmd_single = " ".join(["~/miniconda3/bin/fastp --in1", big_single, "-o", big_single_out, 
    "-w", str(threads), "-A"])
    cmd_paired = " ".join(["~/miniconda3/bin/fastp --in1", big_pair_1, "-o", big_pair_1_out,
    "--in2", big_pair_2, "--out2", big_pair_2_out,
    "-w", str(threads), "-A"])

    code = runCommand(cmd_single)
    for i in range(n_tests):
        start = timer()
        code = runCommand(cmd_single)
        end = timer()
        if(code != 0):
            quit()
        duration_single += (end-start)

    code = runCommand(cmd_paired)
    for i in range(n_tests):
        start = timer()
        code = runCommand(cmd_paired)
        end = timer()
        if(code != 0):
            quit()
        duration_paired += (end-start)

    return duration_single / n_tests, duration_paired / n_tests

start_threads = 2
step_threads = 1
end_threads = 4

threads_list = np.arange(start_threads, end_threads+step_threads, step_threads)
#chunks_list = np.arange(start_chunk, end_chunk+step_chunk, step_chunk)

for thread_number in threads_list:
    fastp_single, fastp_paired = run_fastp(thread_number)
    print("-t " + str(4) + " -A: " + str(fastp_single) + "s, " + str(fastp_paired) + "s")
    single_time, paired_time = run_luthien(thread_number, 12)
    print("-t " + str(thread_number) + ": " + str(single_time) + "s, " + str(paired_time) + "s")
#    for chunk_size in chunks_list:
#        
#        print("-t " + str(thread_number) + " -c " + str(chunk_size) + ": "
#            + str(single_time) + "s, " + str(paired_time))
