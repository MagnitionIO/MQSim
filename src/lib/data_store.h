//
// Created by Khubaib Umer on 06/02/2023.
//

#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "base.h"

typedef void store_t;

typedef void data_t;

/// @name new_data_store
/// @brief Creates a new DataStore
/// @return data_store_ptr
EXTERN_C store_t *new_data_store();

/// @name push_head
/// @param store data store
/// @param data user data
/// @brief Add user_data to the head of data_store
EXTERN_C void push_head(store_t *store, data_t *data);

/// @name push_tail
/// @param store data store
/// @param data user data
/// @brief Add user_data to the tail of data_store
EXTERN_C void push_tail(store_t *store, data_t *data);

/// @name pop_head
/// @param store data store
/// @return data_t *user_data
/// @brief Get and remove user_data to the head of data_store
EXTERN_C data_t *pop_head(store_t *store);

/// @name pop_tail
/// @param store data store
/// @return data_t *user_data
/// @brief Get and remove user_data to the tail of data_store
EXTERN_C data_t *pop_tail(store_t *store);

/// @name get_store_size
/// @param store data store
/// @return size_t count
/// @brief Get the number of user_data items in data_store
EXTERN_C size_t get_store_size(store_t *store);

/// @name is_store_empty
/// @param store data store
/// @return bool {TRUE|FALSE}
/// @brief Check if there are any items in data_store
EXTERN_C bool is_store_empty(store_t *store);

/// @name clear_store
/// @param store data store
/// @param free_data {TRUE|FALSE} to indicate if the user_data should be free'd
/// @brief Remove all items from data_store
EXTERN_C void clear_store(store_t *store, bool free_data);

/// @name for_each
/// @param store data store
/// @param func User-Provided Function
/// @brief Calls `func` for each item in data_store
EXTERN_C void for_each(store_t *store, void (*func)(data_t *data));

/// @name find_if
/// @param store data store
/// @param func Predicate function
/// @return data_t *user_data
/// @brief Calls `func` for each item in data_store and returns the user_data when func() returns TRUE
EXTERN_C data_t *find_if(store_t *store, bool (*func)(data_t *data));

#endif //DATA_STORE_H
