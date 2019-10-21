#ifndef _OUTPUT_
#define _OUTPUT_

#include <thread>
#include <fstream>
#include "Config.h"
#include "WorkersHub.h"

using namespace std;

class Output{
public:
    Output();
    void join();
private:
    thread output_thread;
    void output_function();
    void printFromQueue(Job* job);

    ofstream outfile_1;
    ofstream outfile_2;
    ofstream outfile_single;

    bool paired, print_singles;
};

#endif