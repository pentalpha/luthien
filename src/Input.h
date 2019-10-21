#ifndef _INPUT_
#define _INPUT_

#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include "Config.h"
#include "WorkersHub.h"

using namespace std;

class Input{
public:
    Input();
    void join();
private:
    thread input_thread;
    void read_data();
    void input_function();
    void input_function_paired();

    FILE* infile_1;
    FILE* infile_2;

    bool paired;
    long unsigned loaded, chunk_size, max_batch_size;
};

#endif