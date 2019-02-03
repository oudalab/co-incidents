#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <limits>
#include <string>
#include <errno.h>
#include <time.h>
#include <cstring>

#define MAX(a,b) ( ((a) > (b)) ? (a) : (b) )
#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )

#define DOT std::cerr << ".";

// This only works for C
static const char *  currentTime () {
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    char * thetime = asctime(timeinfo);
    thetime[strlen(thetime)-1] = '\0';
    return (const char *) thetime;
    //return asctime(timeinfo);
}

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define DATE_STRING currentTime()

#define log_info(M) std::cerr << DATE_STRING << " [INFO] (" << __FILE__ << ":" << __LINE__ << ") | " << M  << "\n"

#define log_err(M) std::cerr << DATE_STRING << " [ERROR] (" << __FILE__ << ":" << __LINE__ << ": " << std::strerror(errno) << ") | " << M  << "\n"

#define check_mem(A) check((A), "Out of memory.")

void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(),
                [](int ch) { 
                    return !std::isspace(ch);
                }).base(), s.end());
}


#endif  // UTIL_HPP
