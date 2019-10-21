#include "WorkersHub.h"

WorkersHub* WorkersHub::instance = 0;

void WorkersHub::init(int workers){
    instance = new WorkersHub(workers);
}

WorkersHub* WorkersHub::get(){
    return instance;
}

WorkersHub::WorkersHub(int workers){
    work_ended_flag = false;
    std::vector<std::mutex> list(workers);
    mutexes.swap(list);

    for(int i = 0; i < workers; i++){
        queue<Job*> q;
        job_queues.push_back(q);
        job_counter.push_back(0);
    }
}

//increase the number of jobs a worker has completed
void WorkersHub::increaseJobCount(int worker){
    lock_guard<mutex> guard(mutexes[worker]);
    job_counter[worker] += 1;
}

//requests a Job for a specific worker. Returns null when no jobs are available.
Job* WorkersHub::getJob(int worker){
    lock_guard<mutex> guard(mutexes[worker]);
    if(!job_queues[worker].empty()){
        Job* job = job_queues[worker].front();
        job_queues[worker].pop();
        job_counter[worker] += 1;
        return job;
    }
    else{
        return NULL;
    }
}

//passes a job to the hub, where it will be assigned to the worker with less jobs done.
void WorkersHub::giveJob(Job* job){
    int worker = getNextWorker();
    lock_guard<mutex> guard(mutexes[worker]);
    job_queues[worker].push(job);
    job_counter[worker] += 1;
}

int WorkersHub::getNextWorker(){
    int smaller = 0;
    for(int i = 0; i < job_counter.size(); i++){
        if(job_counter[smaller] > job_counter[i]){
            smaller = i;
        }
    }
    return smaller;
}