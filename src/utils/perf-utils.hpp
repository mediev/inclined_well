#ifndef PERF_UTILS_H
#define PERF_UTILS_H

#include <functional>
#include <string>

typedef struct
{
    long time;
    long std_dev;
    long iterations;
} exec_time;

#define DUMMY_FUNC [](){}

exec_time measure_time(std::function<void (void)> do_before, std::function<void (void)> func, int n = 10);
exec_time measure_time(std::function<void (void)> func, int n = 10);

std::pair<exec_time, exec_time> measure_time2(std::function<void (void)> do_before_func1, std::function<void (void)> func1, std::function<void (void)> do_before_func2, std::function<void (void)> func2, int n = 10);
std::pair<exec_time, exec_time> measure_time2(std::function<void (void)> func1, std::function<void (void)> func2, int n = 10);

void print_test_results(std::string test1, exec_time time1, std::string test2, exec_time time2);
void print_test_results(std::string test1, exec_time time1);
void print_test_title(std::string s);

#endif /* PERF_UTILS_H */
