#ifndef _WORKERS_HUB_
#define _WORKERS_HUB_

#include <vector>
#include <queue>
#include <mutex>
#include "Job.h"

/**
 * author   Pitágoras Alves
 *
 * This class must store the Jobs and distribute them to the workers as they request it.
 * It is entirely multi-threading safe.
 */

class WorkersHub{
public:
    bool work_ended_flag;

    static void init(int workers);
    static WorkersHub* get();

    WorkersHub(int workers);
    //increase the number of jobs a worker has completed
    void increaseJobCount(int worker);
    //requests a Job for a specific worker. Returns null when no jobs are available.
    Job* getJob(int worker);
    //passes a job to the hub, where it will be assigned to the worker with less jobs done.
    void giveJob(Job* job);

private:
    //a separate queue for each worker
    vector<queue<Job*> > job_queues;
    //a queue for the output thread;
    queue<Job*> output_queue;
    //keeps track of how many jobs each worker has completed
    vector<int> job_counter;
    //one mutex for each worker
    vector<mutex> mutexes;
    mutex output_mutex;

    static WorkersHub* instance;

    //returns the index of the worker with less jobs completed
    int getNextWorker();
};

#endif