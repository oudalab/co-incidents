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

int dotProduct(int *vect_A, int *vect_B)
{

    int product = 0;

    // Loop for calculate cot product
    for (int i = 0; i < EMBED_SIZE; i++)

        product = product + vect_A[i] * vect_B[i];
    return product;
}

double vectorLength(int *vect)
{
    double length = 0.0;
    for (int i = 0; i < EMBED_SIZE; i++)
    {
        length = length + (vect[i] * 1.0) * (vect[i] * 1.0);
    }
    return sqrt(length);
}


#endif  // UTIL_HPP
