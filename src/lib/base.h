//
// Created by Khubaib Umer on 19/01/2023.
//

#ifndef BASE_H
#define BASE_H

#ifndef __cplusplus
#ifndef bool
#define bool uint8_t
#endif
#define true 1
#define false 0
#define nullptr NULL
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__APPLE__)

#include <pthread.h>

#elif defined(__linux__)
#include <unistd.h>
#include <syscall.h>
#endif

#define return_if(x, y) ({ if (x) {return y;} })
#define _calloc(cnt, typ) (typ*) calloc(cnt, sizeof(typ))

#define STATEFUL_CALL(base_ptr, func, ...) base_ptr->func(base_ptr, __VA_ARGS__)
#define STATEFUL_CALL0(base_ptr, func) base_ptr->func(base_ptr)
#define STATELESS_CALL(base_ptr, func, ...) base_ptr->func(__VA_ARGS__)
#define STATELESS_CALL0(base_ptr, func) base_ptr->func()

#ifdef __cplusplus
#define EXTERN_C extern "C"
#define EXTERN_C_BLOCK_START extern "C" {
#define EXTERN_C_BLOCK_END }
#else
#define EXTERN_C
#endif

typedef struct {
    uint64_t id;
    uint64_t arrival_time;
    uint64_t delay;
    uint64_t response_time;
    uint64_t completion_time;
} completion_info_t;

typedef void (*insert_to_queue_f)(void *datastore, completion_info_t *info);

typedef uint64_t tid_t;

typedef uint64_t request_id_t;

static tid_t get_tid() {
#if defined (__linux__)
    return syscall(SYS_gettid);
#elif defined(__APPLE__)
    tid_t tid;
    pthread_threadid_np(NULL, &tid);
    return tid;
#endif
}

static void do_assert_fail(const char *fl, const char *fn, const int ln, const char *expr) {
    fprintf(stderr, "Failed Assertion: [%llu] : {%s:%d} %s > [ %s ]\n", get_tid(), fl, ln, fn, expr);
}

#define do_assert(expr) if (!(expr)) do_assert_fail(__FILE_NAME__, __func__, __LINE__, #expr)

#endif //BASE_H
