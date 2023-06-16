//
// Created by Khubaib Umer on 16/02/2023.
//

#ifndef SAFE_UNORDERED_MAP_H
#define SAFE_UNORDERED_MAP_H

#include <pthread.h>
#include <unordered_map>

template<typename KeyType, typename ValueType>
class safe_unorderd_map : public std::unordered_map<KeyType, ValueType> {
public:
    safe_unorderd_map() : std::unordered_map<KeyType, ValueType>() {
        pthread_rwlock_init(&lock, nullptr);
    }

    void AddItem(KeyType key, ValueType value) {
        pthread_rwlock_wrlock(&lock);
        std::unordered_map<KeyType, ValueType>::emplace(key, value);
        pthread_rwlock_unlock(&lock);
    }

    void RemoveItem(KeyType key) {
        pthread_rwlock_wrlock(&lock);
        std::unordered_map<KeyType, ValueType>::erase(key);
        pthread_rwlock_unlock(&lock);
    }

    std::pair<KeyType, ValueType> find(KeyType key) {
        pthread_rwlock_wrlock(&lock);
        const auto &it = std::unordered_map<KeyType, ValueType>::find(key);
        pthread_rwlock_unlock(&lock);
        return ((it != this->end()) ? std::make_pair(it->first, it->second) : std::make_pair(0, nullptr));
    }

private:
    pthread_rwlock_t lock{};
};

#endif //SAFE_UNORDERED_MAP_H
