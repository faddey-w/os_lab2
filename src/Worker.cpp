#include <cassert>
#include <chrono>
#include "Worker.h"


void Worker::run_worker() {
    assert(worker_func);

    // Sometimes threads are not receive an id immediately.
    // Since users of this class may rely on get_thread_id()
    // we should wait until the thread will receive its id.
    int cnt = 0;
    while (std::this_thread::get_id() != _thread.get_id()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        cnt += 1; // guard against hanging
        assert(cnt < 1000);
    }
    try {
        worker_func();
    } catch (const ExecutorKilled&) {}
    mtx.lock();
    finished = true;
    mtx.unlock();
}

Worker::Worker(std::function<void()> worker_func)
    : worker_func(worker_func), mtx()
    , _thread(&Worker::run_worker, this) {
    assert(worker_func);
};

bool Worker::is_finished() {
    mtx.lock();
    bool result = finished;
    mtx.unlock();
    return result;
}

void Worker::wait() {
    if (_thread.joinable()) _thread.join();
}

std::thread::id Worker::get_thread_id() const {
    return _thread.get_id();
}

Worker::~Worker() {
    wait();
}

