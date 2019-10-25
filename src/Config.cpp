#include "Config.h"
#include <thread>

Config* Config::_instance = 0;
chrono::milliseconds Config::wait_time = chrono::milliseconds(5);

mutex Config::log_mutex;

void Config::log(const char* msg){
    lock_guard<mutex> guard(log_mutex);
    cout << msg << endl;
}

void Config::log(string msg){
    lock_guard<mutex> guard(log_mutex);
    cout << msg << endl;
}

void Config::log(string_view msg){
    lock_guard<mutex> guard(log_mutex);
    cout << msg << endl;
}

Config* Config::get(){
    if(_instance){
        return _instance;
    }else{
        _instance = getInstance();
        return _instance;
    }
}

Config* Config::getInstance(){
    return new Config();
}

void Config::pass_values(AnyOption* opt){
    int threads_number = std::thread::hardware_concurrency();
    _instance->input_file_1 = opt->getValue("i1");
    _instance->output_file_1 = opt->getValue("o1");

    if(opt->getValue("i2")){
        _instance->paired = true;
        _instance->input_file_2 = opt->getValue("i2");
        _instance->output_file_2 = opt->getValue("o2");

        if(opt->getValue("s")){
            _instance->output_singles = true;
            _instance->output_file_single = opt->getValue("s");
        }
    }

    if(opt->getValue("t")){
        threads_number = stoi(opt->getValue("t"));
    }

    if(threads_number < 3){
        Config::log("Using 3 threads, the minimum value.");
        threads_number = 3;
    }
    _instance->threads = threads_number - 2;

    if(opt->getValue("c")){
        _instance->chunk_size = stoi(opt->getValue("c"));
    }

    _instance->max_batch_len = _instance->chunk_size * _instance->threads * BATCH_LEN_MULTIPLIER;
    _instance->max_chunks = _instance->threads * BATCH_LEN_MULTIPLIER;
    if(opt->getValue("nc")){
        _instance->max_batch_len = _instance->chunk_size * _instance->threads * stoi(opt->getValue("nc"));
        _instance->max_chunks = _instance->threads * stoi(opt->getValue("nc"));
    }

    if(opt->getValue("b")){
        _instance->max_batch_len = stoi(opt->getValue("b"));
    }

    if(opt->getValue("q")){
        _instance->min_quality = stoi(opt->getValue("q"));
    }

    cout << "Parameters:" << endl;
    cout << "\tFirst input file: " << _instance->input_file_1 << endl;
    cout << "\tFirst output file: "  << _instance->output_file_1 << endl;
    if(_instance->paired) {
        cout << "\tSecond input file: "  << _instance->input_file_1 << endl;
        cout << "\tSecond output file: "  << _instance->output_file_2 << endl;
        if(_instance->output_singles){
            cout << "\tSingles output file: "  << _instance->output_file_single << endl;
        }
    }

    cout << "\tWorker threads: "  << _instance->threads << endl;
    cout << "\tChunk size: "  << _instance->chunk_size << endl;
    cout << "\tQualit threshold: "  << _instance->min_quality << endl;
}

Config::Config(){
    //max_batch_len = D_MAX_BATCH_LEN;
    chunk_size = D_CHUNK_SIZE;
    min_quality = D_MIN_QUALITY;
    threads = D_THREADS;
    max_batch_len = chunk_size * threads * BATCH_LEN_MULTIPLIER;
    if(max_batch_len > D_MAX_BATCH_LEN){
        max_batch_len = D_MAX_BATCH_LEN;
    }
    length_threshold = D_MIN_LENGTH;
    max_chunks = threads * BATCH_LEN_MULTIPLIER;

    input_file_1 = NULL;
    input_file_2 = NULL;
    output_file_1 = NULL;
    output_file_2 = NULL;
    output_file_single = NULL;

    paired = false;
    output_singles = false;
}

