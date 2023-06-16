//
// Created by Khubaib Umer on 19/01/2023.
//

#include "../src/lib/libmqsim.h"
#include "../src/lib/data_store.h"
#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <unistd.h>
#include <dlfcn.h>
#include <assert.h>

// This will be our Driver for libMQSim.so (i.e. LF reactor)

#define THREAD_COUNT 5

std::atomic<uint64_t> g_clock;

std::atomic<uint64_t> g_timer;

uint64_t logical_clock(void)
{
    // this will be part of LF-scheduler
    return g_clock;
}

void create_timer(uint64_t req_time)
{
    // add a request in LF-Scheduler to wake up MQSim when this logical timer is expired
    g_timer = req_time;
}

void* time_utility(void *args)
{
    while(true)
    {
        g_clock += 1;
        usleep(1);
    }
}

void req_done(void *store, completion_info_t *req_id)
{
    push_tail(store, req_id);
}

typedef bool (*init_f)(const char *);
typedef bool (*process_f)(request_type_t *request, uint64_t *required_time, insert_to_queue_f queue);

typedef uint64_t (*sim_time_f) (void);

void *routine(void *args)
{
    system_t *handle = nullptr;
    init_system(&handle, "/usr/local/share/libMQSim/ssdconfig.xml");
    print_handle(handle);
    request_type_t dummy_request[2];
    dummy_request[0].req_id = 1;
    dummy_request[0].arrival_time = 16187000;
    dummy_request[0].dev_id = 0;
    dummy_request[0].start_sector_addr = 34567136;
    dummy_request[0].size_in_sectors = 64;
    dummy_request[0].type = READ;

    dummy_request[1].req_id = 2;
    dummy_request[1].arrival_time = 18256000;
    dummy_request[1].dev_id = 2;
    dummy_request[1].start_sector_addr = 11813968;
    dummy_request[1].size_in_sectors = 32;
    dummy_request[1].type = READ;

    uint64_t timer = 0, current_time = dummy_request[0].arrival_time;
    int cnt = 0;
    bool ret = true;
    request_type_t *dummy = nullptr;
    while (ret) {
        dummy = (cnt < (sizeof (dummy_request) / sizeof (*dummy))) ? &dummy_request[cnt] : nullptr;
        current_time = (cnt == 0) ? dummy->arrival_time : timer;
        fprintf (stdout,    "Processing Event:%d current_time:%llu requested_time:%llu\n",
                 cnt, current_time, timer);
        ret = process_request(handle, dummy, &timer, current_time, &req_done);
        dummy = nullptr;
        ++cnt;
    }
    printf("Time Taken: %llu\nReq Cnt %d\n", getSimTime(handle) - dummy_request[0].arrival_time, cnt);
    dump_results(handle, getSimTime(handle));
    return NULL;
}

int main(int argc, char **argv)
{
    create_logger("TestMQ", "/tmp");
    set_max_file_size(-1);
    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_create(&threads[i], NULL, routine, NULL);
    }

    for (int i = 0; i < THREAD_COUNT; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
/*
int main(int argc, char **argv)
{
    void *lib = dlopen("/Users/khubaibumer/git/LF-MQSim/libMQSim.so", RTLD_NOW);
    assert(lib);

    init_f init = (bool(*)(const char*))dlsym(lib, "init_system");

    process_f process = (bool(*)(request_type_t *request, uint64_t *required_time, insert_to_queue_f queue)) dlsym(lib, "process_request");

    sim_time_f sim_time = (sim_time_f) dlsym(lib, "getSimTime");

    assert(init);
    assert(process);
    init("/Users/khubaibumer/git/LF-MQSim/ssdconfig.xml");
    request_type_t dummy_request;
    dummy_request.req_id = 1;
    dummy_request.type = WRITE;
    dummy_request.arrival_time = 938513000;
    dummy_request.dev_id = 4;
    dummy_request.start_sector_addr = 264719034;
    dummy_request.size_in_sectors = 16;
    uint64_t timer= 0;
    int cnt = 0;
    request_type_t *dummy = &dummy_request;
    while (process(dummy, &timer, &func))
    {
        dummy = nullptr;
        ++cnt;
    }

    printf("Time Taken: %llu\nReq Cnt %d\n", sim_time() - dummy_request.arrival_time, cnt);

//    while(1)
//        sleep(60); // block main thread
    return 0;
}
*/
