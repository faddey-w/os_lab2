#include <iostream>
#include <cstdlib>
#include <fstream>
#include "MultithreadExecutor.h"


struct TestResult {
    int a, b, c, d;
    bool succeeded;
};


class Lab2Executor : public MultithreadExecutor<TestResult> {

    const int n_tests;

    // reducer section
    std::ostream& stream;

public:

    Lab2Executor(std::ostream& stream, int n_workers, int n_tests_per_worker)
        : MultithreadExecutor<TestResult>(n_workers)
        , stream(stream), n_tests(n_tests_per_worker) {};

protected:

    void setup() {
        srand(time(0));
    }

    void work() {
        for(int i = 0; i < n_tests; i++) {
            int a = 1 + rand() % 15,
                b = 1 + rand() % 15,
                c = 1 + rand() % 100,
                d = 1 + rand() % 100;
            bool succeeded = (a*b == c+d);
            emit({a, b, c, d, succeeded});
        }
    }

    void reduce(TestResult result, int worker_id) {
        stream << "Worker #" << worker_id << ": "
               << result.a << '*' << result.b
               << (result.succeeded ? " == " : " != ")
               << result.c << '+' << result.d << std::endl;
    }

};


int main(int argc, char **argv) {
    if (argc != 4) {
        std::cerr << "Usage: ./os_lab2 <log-file> <n-workers> <n-tests-per-worker>\n";
        return 1;
    }
    std::ostream* out_stream;
    bool out_stream_is_file = false;
    if (std::string(argv[1]) == "-") {
        out_stream = &std::cout;
    } else {
        auto logfile = new std::ofstream(argv[1]);
        if (!logfile->is_open()) {
            std::cerr << "Can not open log file\n";
            delete logfile;
            return 2;
        }
        out_stream_is_file = true;
        out_stream = logfile;
    }
    int n_workers, n_tests_per_worker;
    try {
        n_workers = std::stoi(argv[2]);
        n_tests_per_worker = std::stoi(argv[3]);
    } catch (const std::invalid_argument&) {
        std::cerr << "Can not not parse n-workers or n-tests\n";
        return 3;
    }

    Lab2Executor(*out_stream, n_workers, n_tests_per_worker).run();

    if (out_stream_is_file) {
        ((std::ofstream*)out_stream)->close();
        delete out_stream;
    }

    return 0;
}