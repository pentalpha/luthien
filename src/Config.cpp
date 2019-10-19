#include "Config.h"

Config* Config::_instance = 0;

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

Config::Config(){
    max_batch_len = D_MAX_BATCH_LEN;
    chunk_size = D_CHUNK_SIZE;
    min_quality = D_MIN_QUALITY;
    threads = D_THREADS;

    input_file_1 = NULL;
    input_file_2 = NULL;
    output_file_1 = NULL;
    output_file_2 = NULL;
    output_file_single = NULL;

    paired = false;
    output_singles = false;
}

void Config::pass_values(AnyOption* opt){
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
        _instance->threads = stoi(opt->getValue("t"));
    }

    if(opt->getValue("c")){
        _instance->chunk_size = stoi(opt->getValue("c"));
    }

    if(opt->getValue("b")){
        _instance->max_batch_len = stoi(opt->getValue("b"));
    }

    if(opt->getValue("q")){
        _instance->min_quality = stoi(opt->getValue("q"));
    }

    cout << _instance->input_file_1 << endl;
    cout << _instance->output_file_1 << endl;
    if(_instance->paired) {
        cout << _instance->input_file_1 << endl;
        cout << _instance->output_file_2 << endl;
        if(_instance->output_singles){
            cout << _instance->output_file_single << endl;
        }
    }

    cout << _instance->threads << endl;
    cout << _instance->chunk_size << endl;
    cout << _instance->max_batch_len << endl;
    cout << _instance->min_quality << endl;
}