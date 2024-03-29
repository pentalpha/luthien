#ifndef _CONFIG_
#define _CONFIG_

#include <iostream>
#include <chrono>
#include <mutex>
#include <string>
#include <iostream>
#include <string_view>
#include "anyoption.h"

using namespace std;

#define MAX_MAX_BATCH_LEN 2000
#define MIN_MAX_BATCH_LEN 16
#define BATCH_LEN_MULTIPLIER 2
#define D_MAX_BATCH_LEN 512

#define MIN_CHUNK_SIZE 1
#define MAX_CHUNK_SIZE 32
#define D_CHUNK_SIZE 12

#define MIN_MIN_QUALITY 1
#define MAX_MIN_QUALITY 100
#define D_MIN_QUALITY 20

#define D_THREADS 3

#define D_MIN_LENGTH 20

#define SANGER_OFFSET 33
#define SANGER_MAX 126

#define ILLUMINA_OFFSET 64
#define ILLUMINA_MAX 110

#define SOLEXA_OFFSET 64
#define SOLEXA_MIN 58
#define SOLEXA_MAX 112

class Config{
public:
    static Config* get();
    static void pass_values(AnyOption* opt);

    int max_batch_len;
    int chunk_size;
    int max_chunks;
    int length_threshold;

    int min_quality, quality_offset;
    int threads;

    char* input_file_1;
    char* input_file_2;
    char* output_file_1;
    char* output_file_2;
    char* output_file_single;

    bool paired;
    bool output_singles;

    static chrono::milliseconds wait_time;
    static mutex log_mutex;
    static void log(const char* msg);
    static void log(string msg);
    static void log(string_view msg);

private:
    static Config* getInstance();
    static Config* _instance;

    Config();
};

#endif