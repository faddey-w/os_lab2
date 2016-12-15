#ifndef OS_LAB2_TESTMULTITHREADEDEXECUTOR_H
#define OS_LAB2_TESTMULTITHREADEDEXECUTOR_H

#include <cxxtest/TestSuite.h>
#include <src/MultithreadExecutor.h>
#include <iostream>


class ExampleExecutor : public MultithreadExecutor<int> {

    // parameters
    const int n_yields;

    // reducer section
    int last_worker_id = -1;
    long switch_count = 0;
    long items_received = 0;

public:
    ExampleExecutor(int n_workers, int n_yields)
        : MultithreadExecutor(n_workers), n_yields(n_yields) {};

    long get_switch_count() { return switch_count; }
    long get_items_received() { return items_received; }

protected:

    void work() {
        for(int i = 0; i < n_yields; ++i) {
            emit(i);
        }
    }

    void setup() {
        last_worker_id = -1;
        switch_count = 0;
        items_received = 0;
    }

    void reduce(int i, int worker_id) {
        items_received += 1;
        if (worker_id != last_worker_id) {
            switch_count += 1;
            last_worker_id = worker_id;
        }
    }

};


class TestMultithreadedExecutor : public CxxTest::TestSuite {
public:

    void test_asynchrony() {
        ExampleExecutor ex(10, 10000);
        ex.run();
        TS_ASSERT(ex.get_switch_count() > 10);
    }

    void test_stress() {
        int n_workers = 50;
        int n_yields = 50000;
        ExampleExecutor ex(n_workers, n_yields);
        ex.run();
        TS_ASSERT(ex.get_items_received() == long(n_workers) * n_yields);
    }

};



#endif //OS_LAB2_TESTMULTITHREADEDEXECUTOR_H
