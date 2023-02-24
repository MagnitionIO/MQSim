//
// Created by Khubaib Umer on 11/01/2023.
//

#include "integration.h"
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#if defined(__APPLE__)

#include <dispatch/dispatch.h>

#elif defined(__linux__)
#include <semaphore.h>
#endif

typedef struct {
#if defined(__APPLE__)
    dispatch_semaphore_t semaphore;
#elif defined(__linux__)
    sem_t semaphore;
#endif
#ifndef SINGLETON
    clock_cb_f master_clock;
#else
    clock_f master_clock;
#endif
    done_cb_f request_complete;
    done_cb_f request_timer_callback;
    uint64_t id;
    bool init;
} mag_data_t;

static bool g_enable_integration = false;

void mag_init_semaphore(mag_data_t *data) {
#if defined(__APPLE__)
    data->semaphore = dispatch_semaphore_create(0);
#elif defined (__linux__)
    assert(sem_init (&data->semaphore, 0, 0) == 0);
#endif
}

void *magnition_malloc(size_t size) { return calloc(1, size); }

void magnition_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

void free_mag_instance(mag_layer_t *instance) {
    free(instance->controller->internal);
    free(instance->controller);
#ifndef SINGLETON
    free(instance->internal);
    free(instance);
#endif
}

#ifdef SINGLETON

#define MAG_DATA ((mag_data_t*) MAGNITION->internal)

void do_exit() { MAG_DATA->init = false; }

bool is_system_running() { return MAG_DATA->init; }

void set_clock(clock_f clock) { MAG_DATA->master_clock = clock; }

void set_completion_callback(done_cb_f done) { MAG_DATA->request_complete = done; }

void set_timer_callback(done_cb_f timer) { MAG_DATA->request_timer_callback = timer; }

uint64_t get_clock_time() { return MAG_DATA->master_clock(); }

void do_completion_callback(uint64_t id) { MAG_DATA->request_complete(id); }

void do_request_timer_callback(uint64_t id) { MAG_DATA->request_timer_callback(id); }

void notify_waiter() {
#if defined(__APPLE__)
    dispatch_semaphore_signal(MAG_DATA->semaphore);
#elif defined(__linux__)
    sem_post (&MAG_DATA->semaphore);
#endif
}

void wait_for_event() {
#if defined(__APPLE__)
    dispatch_semaphore_wait(MAG_DATA->semaphore, DISPATCH_TIME_FOREVER);
#elif defined(__linux__)
    sem_wait (&MAG_DATA->semaphore);
#endif
}

void mag_destroy_semaphore() {
#if defined(__APPLE__)
    dispatch_release(MAG_DATA->semaphore);
#elif defined(__linux__)
    sem_destroy(&MAG_DATA->semaphore);
#endif
}

void initialize_instance(mag_data_t *data) {
    mag_init_semaphore(data);
    data->init = true;
}

mag_layer_t *get_instance() {
    static mag_data_t g_data = {.init = false};

    static mag_layer_t g_instance = {
            .set_master_clock = &set_clock,
            .get_logical_time = &get_clock_time,
            .set_done_callback = &set_completion_callback,
            .done_callback = &do_completion_callback,
            .register_timer_callback = &set_timer_callback,
            .request_timer_callback = &do_request_timer_callback,
            .notify = &notify_waiter,
            .wait = &wait_for_event,
            .malloc = &magnition_malloc,
            .free = magnition_free,
            .exit = &do_exit,
            .is_running = &is_system_running,
            .internal = &g_data,
    };

    if (!((mag_data_t *) (g_instance.internal))->init) {
        g_instance.controller = new_job_controller();
        initialize_instance(&g_data);
    }
    return &g_instance;
}

#else

#define THIS_CAST(x) ((mag_layer_t*)x)
#define MAG_DATA(x) ((mag_data_t*)(THIS_CAST(x)->internal))

void do_exit(void *self) { MAG_DATA(self)->init = false; }

bool is_system_running(void *self) { return MAG_DATA(self)->init; }

void set_clock(void *self, clock_cb_f clock) { MAG_DATA(self)->master_clock = clock; }

void set_completion_callback(void *self, done_cb_f done) { MAG_DATA(self)->request_complete = done; }

void set_timer_callback(void *self, done_cb_f timer) { MAG_DATA(self)->request_timer_callback = timer; }

uint64_t get_clock_time(void *self) { return MAG_DATA(self)->master_clock(); }

void do_completion_callback(void *self, uint64_t id) { MAG_DATA(self)->request_complete(NULL, id); }

void do_request_timer_callback(void *self, uint64_t id) { MAG_DATA(self)->request_timer_callback(NULL, id); }

void notify_waiter(void *self) {
#if defined(__APPLE__)
    dispatch_semaphore_signal(MAG_DATA(self)->semaphore);
#elif defined(__linux__)
    sem_post (&MAG_DATA->semaphore);
#endif
}

void wait_for_event(void *self) {
#if defined(__APPLE__)
    dispatch_semaphore_wait(MAG_DATA(self)->semaphore, DISPATCH_TIME_FOREVER);
#elif defined(__linux__)
    sem_wait (&MAG_DATA->semaphore);
#endif
}

void mag_destroy_semaphore(void *self) {
#if defined(__APPLE__)
    dispatch_release(MAG_DATA(self)->semaphore);
#elif defined(__linux__)
    sem_destroy(&MAG_DATA->semaphore);
#endif
}

uint64_t get_instance_identity(void *self) {
    return MAG_DATA(self)->id;
}

mag_layer_t *new_integration_layer() {
    mag_data_t *data = _calloc(1, mag_data_t);
    mag_layer_t *instance = _calloc(1, mag_layer_t);

    instance->set_master_clock = &set_clock;
    instance->get_logical_time = &get_clock_time;
    instance->set_done_callback = &set_completion_callback;
    instance->done_callback = &do_completion_callback;
    instance->register_timer_callback = &set_timer_callback;
    instance->request_timer_callback = &do_request_timer_callback;
    instance->notify = &notify_waiter;
    instance->wait = &wait_for_event;
    instance->get_instance_id = &get_instance_identity;
    instance->malloc = &magnition_malloc;
    instance->free = &magnition_free;
    instance->exit = &do_exit;
    instance->is_running = &is_system_running;
    if (!data->init) {
        mag_init_semaphore(data);
        data->id = (long)instance;
        data->init = true;
        instance->internal = data;
    }

    return instance;
}

#endif
