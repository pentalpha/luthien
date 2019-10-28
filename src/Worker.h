#ifndef _WORKER_
#define _WORKER_

#include <thread>
////#include <vector>
#include <assert.h>

#include "Job.h"
#include "Config.h"
#include "WorkersHub.h"


struct SlidingResult{
    int five_prime_cut;
	int three_prime_cut;
};

struct TrimmResults{
    TrimmResults();
    void add(TrimmResults &other);
    void print();
    void print_paired();

    long input_records, kept_p, kept_i1, kept_i2;
    long discard_p, discard_i1, discard_i2;
};

class Worker{
public:
    TrimmResults results;

    Worker(int id);
    void join();
private:
    int worker_id;
    Job* job;
    thread working_thread;
    int quality_offset;
    int length_threshold;
    int qual_threshold;
    bool save_singles;

    bool finished;

    void work_function();
    void process_job();
    bool validate_read(
        string_view header,
        string_view sequence,
        string_view comment,
        string_view qualities);
    string_view c_str_to_view(char* line);

    SlidingResult sliding_window(string_view sequence, string_view qualities);
    int get_quality(char quality_char);
};

#endif