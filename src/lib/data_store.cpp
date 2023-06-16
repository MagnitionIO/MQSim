//
// Created by Khubaib Umer on 06/02/2023.
//

#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <cstdio>
#include "data_store.h"

typedef struct store_node {
    data_t *data;
    struct store_node *next;
    struct store_node *prev;
} store_node_t;

typedef struct data_store {
    store_node_t *head;
    store_node_t *tail;
    size_t count;
    pthread_rwlock_t lk;

    void (*lock)(struct data_store *self);

    void (*unlock)(struct data_store *self);
} data_store_t;

#define DATA_STORE(x) ((data_store_t*)(x))

void lock_store(struct data_store *store) {
    pthread_rwlock_wrlock(&store->lk);
}

void unlock_store(struct data_store *store) {
    pthread_rwlock_unlock(&store->lk);
}

void init_store_lock(struct data_store *store) {
    pthread_rwlock_init(&store->lk, NULL);
    DATA_STORE(store)->lock = &lock_store;
    DATA_STORE(store)->unlock = &unlock_store;
}

store_t *new_data_store() {
    data_store_t *store = _calloc(1, data_store_t);
    init_store_lock(store);
    return store;
}

void push_head(store_t *store, data_t *data) {
    do_assert(store);
    store_node_t *node = _calloc(1, store_node_t);
    node->data = data;
    STATEFUL_CALL0(DATA_STORE(store), lock);
    if (DATA_STORE(store)->head == NULL) {
        DATA_STORE(store)->head = node;
        DATA_STORE(store)->tail = node;
        ++DATA_STORE(store)->count;
        STATEFUL_CALL0(DATA_STORE(store), unlock);
        return;
    }
    store_node_t *old_head = DATA_STORE(store)->head;
    node->next = old_head;
    old_head->prev = node;
    DATA_STORE(store)->head = node;
    ++DATA_STORE(store)->count;
    STATEFUL_CALL0(DATA_STORE(store), unlock);
}

void push_tail(store_t *store, data_t *data) {
    do_assert(store);
    store_node_t *node = _calloc(1, store_node_t);
    node->data = data;
    STATEFUL_CALL0(DATA_STORE(store), lock);
    if (DATA_STORE(store)->head == NULL) {
        DATA_STORE(store)->head = node;
        DATA_STORE(store)->tail = node;
        ++DATA_STORE(store)->count;
        STATEFUL_CALL0(DATA_STORE(store), unlock);
        return;
    }
    DATA_STORE(store)->tail->next = node;
    node->prev = DATA_STORE(store)->tail;
    DATA_STORE(store)->tail = node;
    ++DATA_STORE(store)->count;
    STATEFUL_CALL0(DATA_STORE(store), unlock);
}

data_t *pop_head(store_t *store) {
    return_if(is_store_empty(store), NULL);

    STATEFUL_CALL0(DATA_STORE(store), lock);
    store_node_t *node = DATA_STORE(store)->head;
    DATA_STORE(store)->head = DATA_STORE(store)->head->next;
    void *data = node->data;
    free(node);
    --DATA_STORE(store)->count;
    STATEFUL_CALL0(DATA_STORE(store), unlock);
    return data;
}

data_t *pop_tail(store_t *store) {
    return_if(is_store_empty(store), NULL);

    STATEFUL_CALL0(DATA_STORE(store), lock);
    store_node_t *node = DATA_STORE(store)->tail;
    DATA_STORE(store)->tail = DATA_STORE(store)->tail->prev;
    void *data = node->data;
    free(node);
    --DATA_STORE(store)->count;
    STATEFUL_CALL0(DATA_STORE(store), unlock);
    return data;
}

size_t get_store_size(store_t *store) {
    do_assert(store);
    STATEFUL_CALL0(DATA_STORE(store), lock);
    size_t sz = DATA_STORE(store)->count;
    STATEFUL_CALL0(DATA_STORE(store), unlock);
    return sz;
}

bool is_store_empty(store_t *store) {
    return get_store_size(store) == 0;
}

void clear_store(store_t *store, bool free_data) {
    while (!is_store_empty(store)) {
        data_t *data = pop_tail(store);
        if (free_data) {
            free(data);
        }
    }
}

void for_each(store_t *store, void (*func)(data_t *)) {
    size_t sz = get_store_size(store);
    return_if(sz == 0,);

    STATEFUL_CALL0(DATA_STORE(store), lock);
    store_node_t *node = DATA_STORE(store)->head;
    for (int i = 0; i < sz; ++i) {
        func(node->data);
        node = node->next;
    }
    STATEFUL_CALL0(DATA_STORE(store), unlock);
}

data_t *find_if(store_t *store, bool (*func)(data_t *)) {
    size_t sz = get_store_size(store);
    return_if(sz == 0, NULL);

    STATEFUL_CALL0(DATA_STORE(store), lock);
    store_node_t *node = DATA_STORE(store)->head;
    for (int i = 0; i < sz; ++i) {
        if (func(node->data)) {
            STATEFUL_CALL0(DATA_STORE(store), unlock);
            return node->data;
        }
        node = node->next;
    }
    STATEFUL_CALL0(DATA_STORE(store), unlock);
    return NULL;
}
