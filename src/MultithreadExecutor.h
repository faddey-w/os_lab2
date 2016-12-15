#ifndef OS_LAB2_MULTITHREADEXECUTOR_H
#define OS_LAB2_MULTITHREADEXECUTOR_H

#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

#include "Worker.h"



template<typename Result>
class MultithreadExecutor {

    std::queue<std::pair<Result, int> > results_queue;
    std::vector<Worker*> workers;
    bool killed = false;
    int n_workers;
    std::mutex results_mtx;
    std::condition_variable queue_free;
    std::condition_variable queue_has_data;
    std::mutex kill_mtx;
    std::mutex run_mtx;

    static const int QUEUE_MAX_SIZE = 1000;

public:

    MultithreadExecutor(int n_workers)
        : n_workers(n_workers), results_queue()
        , results_mtx(), kill_mtx(), workers()
        , queue_free() {};

    bool is_killed() {
        kill_mtx.lock();
        bool kld = killed;
        kill_mtx.unlock();
        return kld;
    }

    void kill() {
        kill_mtx.lock();
        killed = true;
        kill_mtx.unlock();
    }

    void wait() {
        run_mtx.lock();
        run_mtx.unlock();
    }

    void run() {
        std::unique_lock<std::mutex> run_lk(run_mtx);

        setup();

        auto call_work = [this] () { this->work(); };
        for(int i = 0; i < n_workers; ++i) {
            workers.push_back(new Worker(call_work));
        }

        bool all_finished;
        auto is_queue_not_empty = [this]() {
            return !results_queue.empty();
        };
        do {
            std::pair<Result, int> item;
            {
                std::unique_lock<std::mutex> lk(results_mtx);
                queue_has_data.wait(lk, is_queue_not_empty);
                item = results_queue.front();
                results_queue.pop();
                queue_free.notify_one();
            }
            reduce(item.first, item.second);

            all_finished = true;
            for(auto w : workers) {
                if (!w->is_finished()) {
                    all_finished = false;
                    break;
                }
            }
        } while (!all_finished);

        // delete workers
        for(auto w : workers) {
            delete w;
        }
        workers.clear();

        // all workers are finished now, so no synchronization needed
        while(!results_queue.empty()) {
            auto item = results_queue.front();
            results_queue.pop();
            reduce(item.first, item.second);
        }

        finalize();
    }

protected:
    virtual void work() = 0;
    virtual void reduce(Result data, int worker_id) = 0;
    virtual void setup() {};
    virtual void finalize() {};

    void emit(Result data) {
        // find out what worker emitted this data
        auto thread_id = std::this_thread::get_id();
        int worker_id = -1;
        for(int i = 0; i < workers.size(); ++i) {
            if (workers[i]->get_thread_id() == thread_id) {
                worker_id = i+1;
                break;
            }
        }
        if (worker_id == -1)
            throw std::runtime_error("emit() should be called from worker");

        {
            std::unique_lock<std::mutex> lk(results_mtx);
            queue_free.wait(lk, [this](){
                return results_queue.size() < QUEUE_MAX_SIZE;
            });
            results_queue.push(std::pair<Result, int>(data, worker_id));
            queue_has_data.notify_one();
        }
    }

    void check_exit() {
        if (is_killed()) throw ExecutorKilled();
    }

};


#endif //OS_LAB2_MULTITHREADEXECUTOR_H
