//
// Created by Khubaib Umer on 19/01/2023.
//

#include "base.h"
#include "JobController.h"
#include <pthread.h>
#include <atomic>
#include <queue>

typedef struct {
    uint64_t id;
    void *ptr;
} job_data_t;

typedef struct {
    pthread_rwlock_t lock;
    std::queue<job_data_t *> jobQ;
} controller_t;

#define CONTROLLER(x) ((controller_t*)((job_controller_t*)x)->internal)

uint64_t create_job_id() {
    static std::atomic<uint64_t> id;
    return ++id;
}

bool add_job_to_queue(void *self, void *job) {
    pthread_rwlock_wrlock(&CONTROLLER(self)->lock);
    job_data_t *ptr = _calloc(1, job_data_t);
    ptr->id = create_job_id();
    ptr->ptr = job;
    CONTROLLER(self)->jobQ.push(ptr);
    pthread_rwlock_unlock(&CONTROLLER(self)->lock);
    return true;
}

void *get_job_from_queue(void *self) {
    job_data_t *data = nullptr;
    pthread_rwlock_wrlock(&CONTROLLER(self)->lock);
    if (!CONTROLLER(self)->jobQ.empty()) {
        data = CONTROLLER(self)->jobQ.front();
        CONTROLLER(self)->jobQ.pop();
    }
    pthread_rwlock_unlock(&CONTROLLER(self)->lock);
    return data;
}

uint64_t get_internal_job_id(void *job) {
    return ((job_data_t *) job)->id;
}

void *get_internal_job_data(void *job) {
    return ((job_data_t *) job)->ptr;
}

void delete_internal_job(void *job) {
    job_data_t *data = (job_data_t *) job;
    data->id = 0;
    data->ptr = 0;
    free(job);
}

controller_t *new_controller() {
    controller_t *cntr = _calloc(1, controller_t);
    pthread_rwlock_init(&cntr->lock, nullptr);
    return cntr;
}

job_controller_t *new_job_controller() {
    job_controller_t *jc = _calloc(1, job_controller_t);
    jc->add_job = &add_job_to_queue;
    jc->get_job = &get_job_from_queue;
    jc->get_job_id = &get_internal_job_id;
    jc->get_job_data = &get_internal_job_data;
    jc->delete_job = &delete_internal_job;
    jc->internal = new_controller();
    return jc;
}
