#include "Worker.h"
#include <iostream>
#include <sstream>

TrimmResults::TrimmResults(){
    input_records = 0;
    kept_i1 = 0;
    kept_i2 = 0;
    kept_p = 0;
    discard_p = 0;
    discard_i1 = 0;
    discard_i2 = 0;
}

void TrimmResults::add(TrimmResults &other){
    input_records += other.input_records;
    kept_i1 += other.kept_i1;
    kept_i2 += other.kept_i2;
    kept_p += other.kept_p;
    discard_p += other.discard_p;
    discard_i1 += other.discard_i1;
    discard_i2 += other.discard_i2;
}

void TrimmResults::print(){
    cout << "Total input FastQ records: " << input_records << "\n";
    cout << "FastQ records kept: " << kept_i1 << "(" << ((float)discard_i1 / (float)input_records)*100 << "%)\n";
    cout << "FastQ records discarded: " << discard_i1 << "\n";
}

void TrimmResults::print_paired(){
    cout << "Total input FastQ records: " << input_records << "\n";
    cout << "FastQ records kept: " << kept_p
        << "(" << ((float)kept_p / (float)input_records)*100 << "%)\n";
    cout << "Fastq single records kept:\n\tin1: " << kept_i1 <<"\n\tin2: " << kept_i2 << "\n";
    cout << "FastQ paired records discarded: " << discard_p << "\n";
    cout << "\tFastQ in1 records discarded: " << discard_i1 << "\n";
    cout << "\tFastQ in2 records discarded: " << discard_i2 << "\n";
}

Worker::Worker(int id){
    worker_id = id;
    job = NULL;
    quality_offset = Config::get()->quality_offset;
    length_threshold = Config::get()->length_threshold;
    qual_threshold = Config::get()->min_quality;
    save_singles = Config::get()->output_singles;
    finished = false;
    working_thread = thread(&Worker::work_function, this);
}

void Worker::join(){
    Config::log("Worker: joining in.");
    if(working_thread.joinable()){
        working_thread.join();
    }
}

void Worker::work_function(){
    Config::log(string("Worker ") + to_string(worker_id) + string(": Starting."));
    while(!WorkersHub::get()->reading_ended_flag){
        job = WorkersHub::get()->getJob(worker_id);
        if(job != NULL){
            //Config::log("Worker: Processing job.");
            process_job();
        }else{
            //Config::log("Worker: Resting until more work is available...");
            this_thread::sleep_for(Config::wait_time);
        }
    }

    //Config::log("Worker: Reading is done, processing remaining jobs.");
    job = WorkersHub::get()->getJob(worker_id);
    while(job != NULL){
        process_job();
        job = WorkersHub::get()->getJob(worker_id);
    }
    finished = true;
    Config::log(string("Worker ") + to_string(worker_id) + string(": Finished."));
}

const char* to_cstr(std::string && s)
{
    static thread_local std::string sloc;
    sloc = std::move(s);
    return sloc.c_str();
}

void Worker::process_job(){
    bool paired = job->paired;
    //assert(job != NULL);
    if(paired){
        //assert(job->lines1.size() == job->lines2.size());
    }
    size_t start_line = 0;

    string_view header1, header2;
    string_view sequence1, sequence2;
    string_view comment1, comment2;
    string_view qualities1, qualities2;

    SlidingResult result1, result2;

    bool read1_passed, read2_passed;
    stringstream *out_str1, *out_str2, *out_strS;
    out_str1 = new stringstream();
    if(paired){out_str2 = new stringstream();}
    if(save_singles){ out_strS = new stringstream();}
    results.input_records += (job->lines1.size() / 4);
    /*out_str1 = new string();
    if(paired){out_str2 = new string();}
    if(save_singles){ out_strS = new string();}*/
    //Config::log(string("Processing log with lines1.size()=")+to_string(job->lines1.size()));
    do{
        //Config::log(string("Worker: Parsing reads from start_line=") + to_string(start_line));
        read1_passed = true;
        header1    = c_str_to_view(job->lines1[start_line+0]);
        sequence1  = c_str_to_view(job->lines1[start_line+1]);
        comment1   = c_str_to_view(job->lines1[start_line+2]);
        qualities1 = c_str_to_view(job->lines1[start_line+3]);
        //Config::log("Worker: Parsed read1");
        if(!validate_read(header1, sequence1, comment1, qualities1)){
            exit(EXIT_FAILURE);
        }

        if(paired){
            read2_passed = true;
            header2    = c_str_to_view(job->lines2[start_line+0]);
            sequence2  = c_str_to_view(job->lines2[start_line+1]);
            comment2   = c_str_to_view(job->lines2[start_line+2]);
            qualities2 = c_str_to_view(job->lines2[start_line+3]);
            if(!validate_read(header2, sequence2, comment2, qualities2)){
                exit(EXIT_FAILURE);
            }
            //Config::log("Worker: Parsed read2");
        }

        //Config::log("Input: Running sliding window");
        result1 = sliding_window(sequence1, qualities1);
        if(paired){
            result2 = sliding_window(sequence2, qualities2);
        }

        //Config::log("Input: Saving results form reads");
        if(result1.five_prime_cut != -1 && result1.three_prime_cut != -1){
            if(result1.five_prime_cut > 0){
                sequence1.remove_prefix(result1.five_prime_cut);
                qualities1.remove_prefix(result1.five_prime_cut);
            }

            if(result1.three_prime_cut < (int)sequence1.length()){
                int to_remove = sequence1.length()-1 -result1.three_prime_cut;
                if(to_remove > 0){
                    sequence1.remove_suffix(to_remove);
                    qualities1.remove_suffix(to_remove);
                }
            }
        }else{
            read1_passed = false;
        }

        if(result2.five_prime_cut != -1 && result2.three_prime_cut != -1){
            if(result2.five_prime_cut > 0){
                sequence2.remove_prefix(result2.five_prime_cut);
                qualities2.remove_prefix(result2.five_prime_cut);
            }

            if(result2.three_prime_cut < (int)sequence2.length()){
                int to_remove = sequence2.length()-1 -result2.three_prime_cut;
                if(to_remove > 0){
                    sequence2.remove_suffix(to_remove);
                    qualities2.remove_suffix(to_remove);
                }
            }
        }else{
            read2_passed = false;
        }

        if((paired && (read1_passed && read2_passed))
            || (!paired && read1_passed)){
            *out_str1 << header1 << "\n"
                << sequence1 << "\n"
                << comment1 << "\n"
                << qualities1 << "\n";
            if(paired){
                *out_str2 << header2 << "\n"
                    << sequence2 << "\n"
                    << comment2 << "\n"
                    << qualities2 << "\n";
                results.kept_p += 1;
            }else{
                results.kept_i1 += 1;
            }
        }else if(paired){
            results.discard_p += 1;
            if(read1_passed){
                results.kept_i1 += 1;
                results.discard_i2 += 1;
                if(save_singles){
                    *out_strS << header1 << "\n";
                    *out_strS << sequence1 << "\n";
                    *out_strS << comment1 << "\n";
                    *out_strS << qualities1 << "\n";
                }
            }else if(read2_passed){
                results.kept_i2 += 1;
                results.discard_i1 += 1;
                if(save_singles){
                    *out_strS << header2 << "\n";
                    *out_strS << sequence2 << "\n";
                    *out_strS << comment2 << "\n";
                    *out_strS << qualities2 << "\n";
                }
            }
        }else{
            results.discard_i1 += 1;
        }
        //Config::log("Input: Finished processing pair of reads.");
        start_line += 4;
    }while(start_line < job->lines1.size());
    job->out_str_1 = out_str1;
    if(paired){
        job->out_str_2 = out_str2;
        if(save_singles){
            job->out_str_single = out_strS;
        }
    }
    for(size_t i = 0; i < job->lines1.size(); i++){
        delete(job->lines1[i]);
    }
    for(size_t i = 0; i < job->lines2.size(); i++){
        delete(job->lines2[i]);
    }
    WorkersHub::get()->giveOutput(job);
    job = NULL;
}

string_view Worker::c_str_to_view(char* line){
    string_view view{line};
    if(view[view.length()-1] == '\n'){
        view.remove_suffix(1);
    }
    return view;
}

bool Worker::validate_read(
    string_view header,
    string_view sequence,
    string_view comment,
    string_view qualities)
{
    if(header.length() <= 1){
        Config::log("[Fastq Validation Error] Header is too short:");
        Config::log(header);
        return false;
    }

    if(header[0] != '@'){
        Config::log("[Fastq Validation Error] All fastq IDs should start with a '@':");
        Config::log(header);
        Config::log(sequence);
        Config::log(comment);
        Config::log(qualities);
        return false;
    }

    if(sequence.length() < 1){
        //error(actual_seq);
        Config::log("[Fastq Validation Error] Sequence line is empty");
        return false;
    }

    if(comment.length() < 1){
        //error(actual_seq);
        Config::log("[Fastq Validation Error] Comment line is empty");
        return false;
    }

    if(qualities.length() != sequence.length()){
        //error(actual_seq);
        Config::log("[Fastq Validation Error] Sequence and quality lines have different lengths:");
        Config::log(sequence);
        Config::log(qualities);
        return false;
    }

    return true;
}

SlidingResult Worker::sliding_window(string_view sequence, string_view qualities){
    int sequence_length = sequence.length();
    SlidingResult result;
    result.three_prime_cut = sequence_length;
    result.five_prime_cut = 0;

    int window_size = (int) (0.1 * sequence_length);
    /* if the seq length is less then 10bp, */
	/* then make the window size the length of the seq */
    if (window_size == 0) window_size = sequence_length;
	//std::cout << "Window size is: " << window_size << "\n";
	int window_start=0;
	int window_total=0;

	int found_five_prime = 0;
    //size_t npos;

    /* discard if the length of the sequence is less than the length threshold */
    if (sequence_length < length_threshold) {
		result.three_prime_cut = -1;
		result.five_prime_cut = -1;
		return result;
	}

    for (int i = 0; i < window_size; i++) {
		window_total += get_quality(qualities[i]);
	}

    double window_avg;

    for (int i=0; i <= (sequence_length - window_size); i++) {
        window_avg = (double)window_total / (double)window_size;

        /* Finding the 5' cutoff */
		/* Find when the average quality in the window goes above the threshold starting from the 5' end */
        if (found_five_prime == 0 && window_avg >= qual_threshold) {
			/* at what point in the window does the quality go above the threshold? */
			for (int j = window_start; j < window_start+window_size; j++) {
				if (get_quality (qualities[j]) >= qual_threshold) {
					result.five_prime_cut = j;
					break;
				}
			}
			found_five_prime = 1;
		}

        /* Finding the 3' cutoff */
		/* if the average quality in the window is less than the threshold */
		/* or if the window is the last window in the read */
		if ((window_avg < qual_threshold || (window_start+window_size) > sequence_length) 
            && (found_five_prime == 1)) {

			/* at what point in the window does the quality dip below the threshold? */
			for (int j=window_start; j < window_start+window_size; j++) {
				if (get_quality (qualities[j]) < qual_threshold) {
					result.three_prime_cut = j;
					break;
				}
			}

			break;
		}

        /* instead of sliding the window, subtract the first qual and add the next qual */
		window_total -= get_quality (qualities[window_start]);
		if (window_start+window_size < sequence_length) {
			window_total += get_quality (qualities[window_start+window_size]);
		}
		window_start++;
    }

    /* if cutting length is less than threshold then return -1 for both */
    /* to indicate that the read should be discarded */
    /* Also, if you never find a five prime cut site, then discard whole read */
    if ((found_five_prime == 0) || ((result.three_prime_cut - result.five_prime_cut) < length_threshold)) {
        result.three_prime_cut = -1;
        result.five_prime_cut = -1;
    }

    return result;
}

int Worker::get_quality(char quality_char){
    return (int)quality_char - quality_offset;
}