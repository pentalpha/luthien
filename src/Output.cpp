#include "Output.h"
#include <assert.h>

Output::Output(){
    paired = Config::get()->paired;
    print_singles = Config::get()->output_singles;

    this->outfile_1.open(Config::get()->output_file_1, std::ofstream::out | std::ofstream::trunc);
    if(paired){
        //Config::log("Paired!");
        //Config::log("Writing to ");
        //Config::log(Config::get()->output_file_2);
        this->outfile_2.open(Config::get()->output_file_2, std::ofstream::out | std::ofstream::trunc);
    }
    if(print_singles){
        this->outfile_single.open(Config::get()->output_file_single, std::ofstream::out | std::ofstream::trunc);
    }

    assert(outfile_1.is_open());

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
    //int chars_saved = 0;
    //Config::log("Writing lines from job ");
    //Config::log(to_string(job->start_at));
    //Config::log(to_string(job->output_lines1.size()));
    /*for(string_view view : job->output_lines1){
        outfile_1 << view << "\n";
        //chars_saved += view.length();
    }

    if(paired){
        for(string_view view : job->output_lines2){
            outfile_2 << view << "\n";
            //chars_saved += view.length();
        }
        if(print_singles){
            for(string_view view : job->output_lines_single){
                outfile_single << view << "\n";
                //chars_saved += view.length();
            }
        }
    }*/
    outfile_1 << job->out_str_1->str();
    delete(job->out_str_1);
    if(paired){
        outfile_2 << job->out_str_2->str();
        delete(job->out_str_2);
        if(print_singles){
            outfile_single << job->out_str_single->str();
            delete(job->out_str_single);
        }
    }

    //Config::log("Deleting lines");
    /*for(unsigned i = 0; i < job->lines1.size(); i++){
        delete(job->lines1[i]);
    }
    if(paired){
        for(unsigned i = 0; i < job->lines2.size(); i++){
            delete(job->lines2[i]);
        }
    }*/
    WorkersHub::get()->increaseProcessedCount(job);
    delete(job);
    //Config::log("Finished writing job");
    //Config::log(to_string(chars_saved));
}

void Output::output_function(){
    Config::log("Output: Starting outputing.");
    while(!WorkersHub::get()->work_ended_flag){
        Job* job = WorkersHub::get()->getOutput();
        if(job != NULL){
            printFromQueue(job);
        }else{
            //Config::log("Output: No output chunk yet, waiting...");
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

    outfile_1.close();
    if(outfile_2.is_open()){
        outfile_2.close();
        if(outfile_single.is_open()){
            outfile_single.close();
        }
    }
}