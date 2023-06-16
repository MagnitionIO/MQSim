//
// Created by Khubaib Umer on 13/02/2023.
//

#ifndef SCOPEDMAPINSERTER_H
#define SCOPEDMAPINSERTER_H

#if !defined(typeof)
#define typeof __typeof__
#endif

#include <string>

/// @name ScopedMapInserter
/// @arg MapType
/// @arg KeyType
/// @arg ValueType
/// @param Map Container MapType to insert into
/// @param Key KeyType to insert against
/// @param Value ValueType to be inserted
/// @brief Inserts ValueType v into the MapType map against ValueType v
///        The value is removed automatically as soon as Scope is lapsed
template<typename MapType, typename KeyType, typename ValueType>
class ScopedMapInserter {
public:
    ScopedMapInserter(MapType &map, KeyType k, ValueType v)
            : key_(k), map_(map) {
        map.AddItem(key_, v);
    }

    ~ScopedMapInserter() {
        map_.RemoveItem(key_);
    }

private:
    const KeyType key_;
    MapType &map_;
};

/// @name SCOPED_INSERTER
/// @param m: Map
/// @param k: Key
/// @param v: Value
/// @brief Creates a ScopedMapInserter instance on stack which is deleted
///        as soon as the scope is lapsed which removes the inserted value
#define SCOPED_INSERTER(m, k, v) ScopedMapInserter<typeof(m) , typeof(k), typeof(v)> obj_##m (m, k, v)

#endif //SCOPEDMAPINSERTER_H
