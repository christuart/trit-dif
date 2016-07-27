#ifndef TIMING_HH
#define TIMING_HH

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#endif

typedef long long int64; typedef unsigned long long uint64;

// Andreas Bonini's timing method from http://stackoverflow.com/a/1861337
/* Returns the amount of milliseconds elapsed since the UNIX epoch. Works on both
 * windows and linux. */
uint64 GetTimeMs64();

float average_historic_time(float _history_times[], int _history_count);
#endif
