#ifndef _JOB_
#define _JOB_

#include <vector>
#include <string_view>

using namespace std;

class Job{
public:
    bool paired = 0;
    unsigned size = 0;
    unsigned start_at = 0;
    vector<char*> lines1;
    vector<char*> lines2;
    vector<string_view> output_lines1;
    vector<string_view> output_lines2;
    vector<string_view> output_lines_single;
};

#endif