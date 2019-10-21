#include "Output.h"

Output::Output(){
    paired = Config::get()->paired;
    print_singles = Config::get()->output_singles;

    this->outfile_1.open(Config::get()->output_file_1);
    if(paired){
        this->outfile_1.open(Config::get()->output_file_2);
    }
    if(print_singles){
        this->outfile_single.open(Config::get()->output_file_single);
    }

    this->output_thread = thread(&Output::output_function, this);
}

void Output::join(){
    if(output_thread.joinable()){
        output_thread.join();
    }else{
        Config::log("Output: not joinable.");
    }
}

void Output::printFromQueue(Job* job){
    if(job != NULL){
        Config::log("Writing lines");
        for(string_view view : job->output_lines1){
            outfile_1 << view << "\n";
        }
        if(paired){
            for(string_view view : job->output_lines2){
                outfile_2 << view << "\n";
            }
            if(print_singles){
                for(string_view view : job->output_lines_single){
                    outfile_single << view << "\n";
                }
            }
        }

        Config::log("Deleting lines");
        for(unsigned i = 0; i < job->lines1.size(); i++){
            delete(job->lines1[i]);
        }
        if(paired){
            for(unsigned i = 0; i < job->lines2.size(); i++){
                delete(job->lines2[i]);
            }
        }
        WorkersHub::get()->increaseProcessedCount(job);
        delete(job);
        Config::log("Deleted lines");
    }else{
        Config::log("Output: No output chunk yet, waiting...");
        this_thread::sleep_for(Config::wait_time);
    }
}

void Output::output_function(){
    Config::log("Output: Starting outputing.");
    while(!WorkersHub::get()->work_ended_flag){
        Job* job = WorkersHub::get()->getOutput();
        if(job != NULL){
            printFromQueue(job);
        }else{
            Config::log("Output: No output chunk yet, waiting...");
            this_thread::sleep_for(Config::wait_time);
        }
    }

    Config::log("Output: Processing is done, printing remaining jobs.");
    Job* job = WorkersHub::get()->getOutput();
    while(job != NULL){
        printFromQueue(job);
        job = WorkersHub::get()->getOutput();
    }

    Config::log("Output: Finished outputing.");
}