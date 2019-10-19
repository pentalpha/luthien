#ifndef _CONFIG_
#define _CONFIG_

#define MAX_MAX_BATCH_LEN 2000
#define MIN_MAX_BATCH_LEN 16
#define D_MAX_BATCH_LEN 512

#define MIN_CHUNK_SIZE 1
#define MAX_CHUNK_SIZE 128
#define D_CHUNK_SIZE 16

#define MIN_MIN_QUALITY 1
#define MAX_MIN_QUALITY 100
#define D_MIN_QUALITY 20

#define D_THREADS 1

#include <iostream>
#include "anyoption.h"

using namespace std;

class Config{
public:
    static Config* get();
    static void pass_values(AnyOption* opt);

    int max_batch_len;
    int chunk_size;

    int min_quality;
    int threads;

    char* input_file_1;
    char* input_file_2;
    char* output_file_1;
    char* output_file_2;
    char* output_file_single;

    bool paired;
    bool output_singles;
private:
    static Config* getInstance();
    static Config* _instance;

    Config();
};

#endif