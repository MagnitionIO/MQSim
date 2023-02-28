//
// Created by Khubaib Umer on 27/02/2023.
//

#ifndef SCOPEDGLOBAL_H
#define SCOPEDGLOBAL_H

typedef void (*constructor_f)(void *);

typedef void (*destructor_f)(void *);

template<typename T>
class ScopedObject {
public:
    ScopedObject(T arg, constructor_f ctr, destructor_f dtr)
            : _obj(arg), _ctr(ctr), _dtr(dtr) {
        _ctr(_obj);
    }

    ~ScopedObject() {
        _dtr(_obj);
    }

private:
    T _obj;
    constructor_f _ctr;
    destructor_f _dtr;
};

#define SCOPED_OBJECT(o, ctr, dtr) ScopedObject<typeof(o)> scoped_##ctr_##dtr(o, ctr, dtr)


#endif //SCOPEDGLOBAL_H
