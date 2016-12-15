#ifndef OS_LAB2_WORKER_H
#define OS_LAB2_WORKER_H

#include <stdexcept>
#include <thread>


class ExecutorKilled: public std::runtime_error {
public:
    ExecutorKilled() : std::runtime_error("") {};
};


class Worker {

    // order of members is important!
    std::function<void()> worker_func;
    bool finished = false;
    std::mutex mtx;
    std::thread _thread;

    void run_worker();

public:

    Worker(std::function<void()> worker_func);

    bool is_finished();

    void wait();

    std::thread::id get_thread_id() const;

    ~Worker();

};


#endif //OS_LAB2_WORKER_H
