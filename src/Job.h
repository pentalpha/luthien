#ifndef _JOB_
#define _JOB_

#include <vector>
#include <string_view>
#include <sstream>

using namespace std;

class Job{
public:
    bool paired = 0;
    unsigned size = 0;
    unsigned start_at = 0;
    vector<char*> lines1;
    vector<char*> lines2;
    //vector<string_view> output_lines1;
    //vector<string_view> output_lines2;
    //vector<string_view> output_lines_single;
    stringstream* out_str_1;
    stringstream* out_str_2;
    stringstream* out_str_single;
};

#endif