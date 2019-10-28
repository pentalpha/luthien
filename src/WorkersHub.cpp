#include "WorkersHub.h"

using namespace std;

WorkersHub* WorkersHub::instance = 0;

void WorkersHub::init(int workers){
    instance = new WorkersHub(workers);
}

WorkersHub* WorkersHub::get(){
    return instance;
}

WorkersHub::WorkersHub(int workers){
    work_ended_flag = false;
    reading_ended_flag = false;
    processed = 0;
    std::vector<std::mutex> list(workers);
    mutexes.swap(list);

    for(int i = 0; i < workers; i++){
        queue<Job*> q;
        job_queues.push_back(q);
        job_counter.push_back(0);
    }
    next_queue = 0;
}

void WorkersHub::increaseProcessedCount(Job* job){
    processed += job->size;
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

Job* WorkersHub::getOutput(){
    lock_guard<mutex> guard(output_mutex);
    if(!output_queue.empty()){
        Job* job = output_queue.front();
        output_queue.pop();
        return job;
    }else{
        return NULL;
    }
}

//passes a job to the hub, where it will be assigned to the worker with less jobs done.
void WorkersHub::giveJob(Job* job){
    size_t worker = getNextWorker();
    lock_guard<mutex> guard(mutexes[worker]);
    job_queues[worker].push(job);
    job_counter[worker] += 1;
}

void WorkersHub::giveOutput(Job* job){
    lock_guard<mutex> guard(output_mutex);
    output_queue.push(job);
}

bool WorkersHub::allQueuesEmpty(){
    for(unsigned i = 0; i < job_queues.size(); i++){
        if(!job_queues[i].empty()){
            return false;
        }
    }
    return true;
}

size_t WorkersHub::getNextWorker(){
    size_t id = next_queue;
    next_queue = (next_queue+1) % job_queues.size();
    return id;
}

void WorkersHub::recycle(Job* job){
    lock_guard<mutex> guard(recycle_mutex);
    recycle_queue.push(job);
}

Job* WorkersHub::getUsedJob(){
    if(recycle_queue.empty()){
        return NULL;
    }else{
        lock_guard<mutex> guard(recycle_mutex);
        Job* job = recycle_queue.front();
        recycle_queue.pop();
        return job;
    }
}
