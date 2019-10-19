#ifndef _MAIN_
#define _MAIN_

#include <sstream>
#include <iostream>
#include <string>
#include <cstdarg>

/*const char* str(const char *fmt, ...){
    std::stringstream builder;
    va_list args;
    va_start(args, fmt);

    while (*fmt != '\0') {
        if (*fmt == 'd') {
            int i = va_arg(args, int);
            builder << std::to_string(i);
        } else if (*fmt == 's') {
            char * s = va_arg(args, char*);
            builder << s;
        }
        ++fmt;
    }

    va_end(args);
    return builder.str().c_str();
}*/

#endif