//
// Created by Khubaib Umer on 11/01/2023.
//

#ifndef INTEGRATION_H
#define INTEGRATION_H

#include "base.h"
#include "JobController.h"
#include <stdlib.h>

typedef void (*done_cb_f)(uint64_t);

#ifndef SINGLETON
#define SINGLETON
#endif

#ifdef SINGLETON

typedef uint64_t (*clock_f)(void);

typedef void (*reg_clock_f)(clock_f);

typedef void (*reg_done_cb_f)(done_cb_f);


typedef struct {
    reg_clock_f set_master_clock;
    clock_f get_logical_time;
    reg_done_cb_f set_done_callback;
    done_cb_f done_callback;
    reg_done_cb_f register_timer_callback;
    done_cb_f request_timer_callback;

    void (*notify)(void); // post event to blocked thread
    void (*wait)(void); // blocks until an event is posted
    void *(*malloc)(size_t size);

    void (*free)(void *ptr);

    void (*exit)(void);

    bool (*is_running)(void);

    job_controller_t *controller;
    // For internal use
    void *internal;
} mag_layer_t;

mag_layer_t *get_instance();

static mag_layer_t *get_mag_integration() { return get_instance(); }

#define MAGNITION get_mag_integration()

#else

typedef uint64_t (*clock_f)(void *self);

typedef uint64_t (*clock_cb_f)(void);

typedef void (*reg_clock_f)(void *self, clock_cb_f);

typedef void (*reg_done_cb_f)(void *self, done_cb_f);

// mag_layer_t will be a singleton instance
typedef struct {
    reg_clock_f set_master_clock;
    clock_f get_logical_time;
    reg_done_cb_f set_done_callback;
    done_cb_f done_callback;
    reg_done_cb_f register_timer_callback;
    done_cb_f request_timer_callback;

    void (*notify)(void *self); // post event to blocked thread
    void (*wait)(void *self); // blocks until an event is posted
    uint64_t (*get_instance_id)(void *self);

    void (*exit)(void *self);

    bool (*is_running)(void *self);

    void *(*malloc)(size_t size);

    void (*free)(void *ptr);

    job_controller_t *controller;

    // For internal use
    void *internal;
} mag_layer_t;

mag_layer_t *new_integration_layer();

#endif

void free_mag_instance(mag_layer_t *instance);

#define INTEGRATION_BIT bool g_integration_enabled
#define ENABLE_INTEGRATION_MODE g_integration_enabled = true

#define CHECK_INTEGRATION (g_integration_enabled == true)

#endif //INTEGRATION_H
