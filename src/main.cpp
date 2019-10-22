#include <iostream>
#include <string>
#include <experimental/filesystem>
#include <thread>
#include "WorkersHub.h"
#include "main.h"
#include "anyoption.h"
#include "Config.h"
#include "Output.h"
#include "Input.h"
#include "Worker.h"

using namespace std;

AnyOption* get_args(int argc, char** argv);
string test_int(const char* arg_name, AnyOption* opt, int minimum, int maximum);
string test_int(const char* int_str, int minimum, int maximum);
string test_invalid_arguments(AnyOption* opt);
void run(AnyOption* opt);

int main(int argc, char** argv){
    AnyOption* opt = get_args(argc, argv);
    if (!opt->hasOptions()) { /* print usage if no options */
        opt->printUsage();
        delete opt;
        return 0;
    }

    string msg = test_invalid_arguments(opt);
    if(msg.length()){
        cout << msg << "\n";
    }else{
        Config::pass_values(opt);
        cout << "Arguments checked" << endl;
        run(opt);
    }

    delete opt;
    return 0;
}

void run(AnyOption* opt){
    int threads = Config::get()->threads;
    WorkersHub::init(threads);

    Input input;
    vector<Worker*> comrades;
    for(int i = 0; i < threads; i++){
        comrades.push_back(new Worker(i));
    }
    Output output;

    input.join();
    WorkersHub::get()->reading_ended_flag = true;
    Config::log("Finished reading inputs.");

    for(int i = 0; i < threads; i++){
        comrades[i]->join();
    }

    WorkersHub::get()->work_ended_flag = true;
    Config::log("Finished processing inputs.");

    output.join();
}

AnyOption* get_args(int argc, char** argv){
    AnyOption *opt = new AnyOption();
    opt->noPOSIX();
    opt->setVerbose();
    opt->autoUsagePrint(true);

    opt->addUsage("usage: ");
    opt->addUsage("\nrequired:");
    opt->addUsage("-i1\tThe first fastq input file.");
    opt->addUsage("-i2\tThe second fastq input file, enables paired end mode.");
    opt->addUsage("-o1\tThe first fastq output file.");
    opt->addUsage("-o2\tThe second fastq output file, required only for paired end mode.");
    opt->addUsage("\noptional:");
    opt->addUsage("-s\tOutput single reads.");
    opt->addUsage("-t\tNumber of worker threads. Maximum: Total number of cores.");
    opt->addUsage(string("\t\tDefault:") + to_string(Config::get()->threads));
    opt->addUsage("-c\tLuthien reads the input files in chunks with this maximum length.");
    opt->addUsage(string("\t\tDefault:") + to_string(Config::get()->chunk_size) + string("MB"));
    opt->addUsage("-b\tMaximum chars from fastq to store in memory.");
    opt->addUsage(string("\t\tDefault:") + to_string(Config::get()->max_batch_len) + string("MB"));
    opt->addUsage("-nc\tThe number of chunks to load for each thread.");
    opt->addUsage("-q\tMinimum quality score.");
    opt->addUsage(string("\t\tDefault:") + to_string(Config::get()->min_quality));
    opt->addUsage("-h --help\tPrint this help.");

    opt->setOption("i1"); opt->setOption("i2");
    opt->setOption("o1"); opt->setOption("o2");

    opt->setOption("s");
    opt->setOption("t");
    opt->setOption("c");
    opt->setOption("b");
    opt->setOption("q");
    opt->setOption("nc");
    opt->setOption("help");

    opt->processCommandArgs(argc, argv);

    return opt;
}

string test_int(const char* arg_name, AnyOption* opt, int minimum, int maximum){
    if(opt->getValue(arg_name)){
        char* value = opt->getValue(arg_name);
        return test_int(value, minimum, maximum);
    }else{
        return "";
    }
}

string test_int(const char* int_str, int minimum, int maximum){
    try {
        int value = std::stoi(int_str);
        if(value < minimum){
            return string("Below minimum value: ")
                + to_string(value) + string(" < ") + to_string(minimum);
        }else if(value > maximum){
            return string("Above maximum value: ")
                + to_string(value) + string(" > ") + to_string(maximum);
        }else{
            return "";
        }
    }catch (const std::invalid_argument& ia) {
        std::cerr << "Invalid argument: " << ia.what() << std::endl;
        return string("Error parsing value ") + string(int_str);
    }catch (const std::out_of_range& oor) {
        std::cerr << "Out of Range error: " << oor.what() << std::endl;
        return string("Error parsing value ") + string(int_str);
    }catch (const std::exception& e) {
        std::cerr << "Undefined error: " << e.what() << std::endl;
        return string("Error parsing value ") + string(int_str);
    }
}

string test_invalid_arguments(AnyOption* opt){
    //checking IO files
    if(!opt->getValue("i1")){
        return "Missing first input file (i1)";
    }else{
        char* i1 = opt->getValue("i1");
        if(!std::experimental::filesystem::exists(i1)){
            return string(i1) + string(" does not exist.");
        }else{
            if(!opt->getValue("o1")){
                return string("Missing first output file (o1)");
            }else{
                std::experimental::filesystem::path o1_file(opt->getValue("o1"));
                auto o1 = std::experimental::filesystem::absolute(o1_file);
                auto parent = o1.parent_path();
                if(!std::experimental::filesystem::exists(parent)){
                    return string(o1.c_str())+string(" parent directory does not exist.");
                }
            }

            char* i2 = opt->getValue("i2");
            if(i2 != NULL){
                if(std::experimental::filesystem::exists(i2)){
                    if(!opt->getValue("o2")){
                        return string("Missing second output file (o1)");
                    }else{
                        std::experimental::filesystem::path o2_file(opt->getValue("o2"));
                        auto o2 = std::experimental::filesystem::absolute(o2_file);
                        auto parent = o2.parent_path();
                        if(!std::experimental::filesystem::exists(parent)){
                            return string(o2.c_str())+ string(" parent directory does not exist.");
                        }
                    }

                    if(opt->getValue("s")){
                        std::experimental::filesystem::path s_file(opt->getValue("s"));
                        auto s = std::experimental::filesystem::absolute(s_file);
                        auto parent = s.parent_path();
                        if(!std::experimental::filesystem::exists(parent)){
                            return string(s.c_str())+ string(" parent directory does not exist.");
                        }
                    }

                }else{
                    return string(i2)+string(" does not exist.");
                }
            }
        }
    }

    //parse optional arguments
    string threads_test = test_int("t", opt, 1, thread::hardware_concurrency());
    if(threads_test.length()){ return threads_test; }

    string chunk_test = test_int("c", opt, MIN_CHUNK_SIZE, MAX_CHUNK_SIZE);
    if(chunk_test.length()){ return chunk_test; }

    string batch_test = test_int("b", opt, MIN_MAX_BATCH_LEN, MAX_MAX_BATCH_LEN);
    if(batch_test.length()){ return batch_test; }

    if(opt->getValue("b")){
        char* b_str = opt->getValue("b");
        int b = stoi(b_str);
        int c = Config::get()->chunk_size;
        if(opt->getValue("c")){
            c = stoi(opt->getValue("c"));
        }

        if(b <= c){
            return string("The value of -b must be greater than -c: '-b'=") 
                + to_string(b) + string(" and '-c'=") + to_string(c);
        }
    }

    string quality_test = test_int("q", opt, MIN_MIN_QUALITY, MAX_MIN_QUALITY);
    if(quality_test.length()){ return quality_test; }
    string queue_len = test_int("nc", opt, 2, 10);
    if(queue_len.length()){ return queue_len; }

    return "";
}