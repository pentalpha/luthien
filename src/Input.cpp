#include "Input.h"

Input::Input(){
    paired = Config::get()->paired;
    loaded = 0;
    chunk_size = Config::get()->chunk_size*(1024*1024);
    max_batch_size = Config::get()->max_batch_len*(1024*1024);

    infile_1 = fopen(Config::get()->input_file_1, "r");
    if (paired){
        infile_2 = fopen(Config::get()->input_file_2, "r");
    }else{
        infile_2 = NULL;
    }
    previous_size = 0;
    input_thread = thread(&Input::read_data, this);
}

void Input::join(){
    if(input_thread.joinable()){
        input_thread.join();
    }else{
        Config::log("Output: not joinable.");
    }
}

void Input::read_data(){
    Config::log("Input: Starting to read input files.");
    if(paired){
        input_function_paired();
    }else{
        input_function();
    }

    Config::log("Input: Closing input files.");
    fclose(infile_1);

    if(infile_2){
        fclose(infile_2);
    }
    Config::log("Input: Closed input files.");
}

Job* Input::new_job(){
    Job* job = WorkersHub::get()->getUsedJob();
    if(job != NULL){
        //Config::log("Retrieved used job, resizing it.");
        job->lines1.resize(0);
        job->lines2.resize(0);
        //Config::log("Reseting values");
        job->paired = paired;
        job->size = 0;
        job->out_str_1 = NULL;
        job->out_str_2 = NULL;
        job->out_str_single = NULL;
    }else{
        //Config::log("Returning new job");
        job = new Job();
        if(previous_size != 0){
            job->lines1.reserve(previous_size);
            if(paired){
                job->lines2.reserve(previous_size);
            }
        }
    }
    //Config::log("Returning job");
    return job;
}

void Input::input_function(){
    long unsigned loaded_chars = 0;
    int current_line = 0;
    vector<char*>* last_remainder = NULL;
    bool eof = false;
    while (!eof) {
        while((loaded_chars-WorkersHub::get()->getProcessed()) >= max_batch_size){
            //Config::log("Input: Loaded too much, waiting for more data to be processed.");
            this_thread::sleep_for(Config::wait_time);
        }

        Job* job = new_job();
        job->paired = false;
        job->start_at = current_line;

        char* line = NULL;
        char *line_1;
        size_t len = chunk_size;
        long loaded = 0;
        long max = (int) chunk_size;

        long len1 = 0;

        if(last_remainder != NULL){
            for(size_t i = 0; i < last_remainder->size(); i++){
                char* line = (*last_remainder)[i];
                len1 = strlen(line);
                loaded += len1;
                job->lines1.push_back(line);
            }
            delete(last_remainder);
            last_remainder = NULL;
        }

        do{
            len1 = getline(&line, &len, infile_1);
            if(len1 < 0){
                eof = true;
                break;
            }
            loaded += len1;
            current_line += 1;
            line_1 = new char[len1+1];
            strncpy(line_1, line, len1);
            line_1[len1] = '\0';

            job->lines1.push_back(line_1);
        }while(loaded < max);

        //job->lines1.shrink_to_fit();

        int extra_lines = job->lines1.size() % 4;

        if(extra_lines > 0 && job->lines1.size() > 0){
            last_remainder = new vector<char*>(extra_lines);
            for(int i = extra_lines-1; i >= 0; i--){
                char* value = job->lines1[job->lines1.size()-1];
                (*last_remainder)[i] = value;
                job->lines1.pop_back();
            }
        }

        job->size = (size_t)(loaded);
        loaded_chars += job->size;
        previous_size = job->lines1.size();

        WorkersHub::get()->giveJob(job);
        //Config::log("Pushed job to WorkersHub");

        if(len1 == -1){
            //Config::log("Input: found end of file.");
            break;
        }
        //Config::log(to_string((int)max_batch_size-((int)loaded_chars-(int)WorkersHub::get()->getProcessed())));
        //Config::log(to_string(loaded_chars));
        //Config::log(to_string(current_line));
    }
}

void Input::input_function_paired(){
    long unsigned loaded_chars = 0;
    int current_line = 0;
    vector<char*>* last_remainder = NULL;
    vector<char*>* last_remainder2 = NULL;
    bool eof = false;
    while (!eof) {
        while((loaded_chars-WorkersHub::get()->getProcessed()) >= max_batch_size){
            //Config::log("Input: Loaded too much, waiting for more data to be processed.");
            this_thread::sleep_for(Config::wait_time);
        }

        Job* job = new_job();
        job->paired = true;
        job->start_at = current_line;

        
        size_t len = chunk_size;
        char* line = new char[chunk_size];
        char* line2 = new char[chunk_size];
        char *line_1, *line_2;
        long loaded = 0;
        long max = (int) chunk_size;

        long len1 = 0;
        long len2 = 0;

        if(last_remainder != NULL){
            for(size_t i = 0; i < last_remainder->size(); i++){
                char* line = (*last_remainder)[i];
                len1 = strlen(line);
                loaded += len1;
                job->lines1.push_back(line);

                char* line2 = (*last_remainder2)[i];
                len2 = strlen(line2);
                loaded += len2;
                job->lines2.push_back(line2);
            }

            delete(last_remainder);
            last_remainder = NULL;

            delete(last_remainder2);
            last_remainder2 = NULL;
        }

        do{

            len2 = getline(&line2, &len, infile_2);
            len1 = getline(&line, &len, infile_1);
            
            if(len1 <= 0 || len2 <= 0){
                eof = true;
                if(len2 > 0 && len1 <= 0){
                    Config::log(string("Input: first input has less lines than the second input,")
                        + string(" ignoring the remaining lines in the first."));
                }else if(len1 > 0 && len2 <= 0){
                    Config::log(string("Input: first input has more lines than the second input,")
                        + string(" ignoring the remaining lines in the second."));
                }
                break;
            }
            loaded += len1;
            loaded += len2;
            current_line += 1;

            line_1 = new char[len1+1];
            strncpy(line_1, line, len1);
            line_1[len1] = '\0';

            line_2 = new char[len2+1];
            strncpy(line_2, line2, len2);
            line_2[len2] = '\0';
            //Config::log(string("First:") + string(line_1));
            //Config::log(string("Second:") + string(line_2));
            job->lines1.push_back(line_1);

            job->lines2.push_back(line_2);
        }while(loaded < max);

        //job->lines1.shrink_to_fit();
        //job->lines2.shrink_to_fit();

        int extra_lines = job->lines1.size() % 4;

        if(extra_lines > 0 && job->lines1.size() > 0){
            last_remainder = new vector<char*>(extra_lines);
            last_remainder2 = new vector<char*>(extra_lines);

            for(int i = extra_lines-1; i >= 0; i--){
                char* value = job->lines1[job->lines1.size()-1];
                (*last_remainder)[i] = value;
                job->lines1.pop_back();

                char* value2 = job->lines2[job->lines2.size()-1];
                (*last_remainder2)[i] = value2;
                job->lines2.pop_back();
            }
        }

        job->size = (size_t)(loaded);
        loaded_chars += job->size;
        previous_size = job->lines1.size();
        //assert(job->lines1.size() == job->lines2.size());
        //assert(job->lines1.size() % 4 == 0);
        WorkersHub::get()->giveJob(job);
        //Config::log("Input: Pushed job to WorkersHub");

        if(len1 == -1){
            //Config::log("Input: found end of file.");
            break;
        }
        //Config::log(to_string((int)max_batch_size-((int)loaded_chars-(int)WorkersHub::get()->getProcessed())));
        //Config::log(to_string(loaded_chars));
        //Config::log(to_string(current_line));
    }
}