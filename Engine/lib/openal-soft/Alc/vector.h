#ifndef AL_VECTOR_H
#define AL_VECTOR_H

#include <stdlib.h>

#include <AL/al.h>

#include "almalloc.h"


#define TYPEDEF_VECTOR(T, N) typedef struct {                                 \
    size_t Capacity;                                                          \
    size_t Size;                                                              \
    T Data[];                                                                 \
} _##N;                                                                       \
typedef _##N* N;                                                              \
typedef const _##N* const_##N;

#define VECTOR(T) struct {                                                    \
    size_t Capacity;                                                          \
    size_t Size;                                                              \
    T Data[];                                                                 \
}*

#define VECTOR_INIT(_x)       do { (_x) = NULL; } while(0)
#define VECTOR_INIT_STATIC()  NULL
#define VECTOR_DEINIT(_x)     do { al_free((_x)); (_x) = NULL; } while(0)

#define VECTOR_RESIZE(_x, _s, _c) do {                                        \
    size_t _size = (_s);                                                      \
    size_t _cap = (_c);                                                       \
    if(_size > _cap)                                                          \
        _cap = _size;                                                         \
                                                                              \
    if(!(_x) && _cap == 0)                                                    \
        break;                                                                \
                                                                              \
    if(((_x) ? (_x)->Capacity : 0) < _cap)                                    \
    {                                                                         \
        size_t old_size = ((_x) ? (_x)->Size : 0);                            \
        void *temp;                                                           \
                                                                              \
        temp = al_calloc(16, sizeof(*(_x)) + sizeof((_x)->Data[0])*_cap);     \
        assert(temp != NULL);                                                 \
        if((_x))                                                              \
            memcpy(((ALubyte*)temp)+sizeof(*(_x)), (_x)->Data,                \
                   sizeof((_x)->Data[0])*old_size);                           \
                                                                              \
        al_free((_x));                                                        \
        (_x) = temp;                                                          \
        (_x)->Capacity = _cap;                                                \
    }                                                                         \
    (_x)->Size = _size;                                                       \
} while(0)                                                                    \

#define VECTOR_CAPACITY(_x) ((_x) ? (_x)->Capacity : 0)
#define VECTOR_SIZE(_x)     ((_x) ? (_x)->Size : 0)

#define VECTOR_BEGIN(_x) ((_x) ? (_x)->Data + 0 : NULL)
#define VECTOR_END(_x)   ((_x) ? (_x)->Data + (_x)->Size : NULL)

#define VECTOR_PUSH_BACK(_x, _obj) do {      \
    size_t _pbsize = VECTOR_SIZE(_x)+1;      \
    VECTOR_RESIZE(_x, _pbsize, _pbsize);     \
    (_x)->Data[(_x)->Size-1] = (_obj);       \
} while(0)
#define VECTOR_POP_BACK(_x) ((void)((_x)->Size--))

#define VECTOR_BACK(_x)  ((_x)->Data[(_x)->Size-1])
#define VECTOR_FRONT(_x) ((_x)->Data[0])

#define VECTOR_ELEM(_x, _o) ((_x)->Data[(_o)])

#define VECTOR_FOR_EACH(_t, _x, _f)  do {                                     \
    _t *_iter = VECTOR_BEGIN((_x));                                           \
    _t *_end = VECTOR_END((_x));                                              \
    for(;_iter != _end;++_iter)                                               \
        _f(_iter);                                                            \
} while(0)

#define VECTOR_FOR_EACH_PARAMS(_t, _x, _f, ...)  do {                         \
    _t *_iter = VECTOR_BEGIN((_x));                                           \
    _t *_end = VECTOR_END((_x));                                              \
    for(;_iter != _end;++_iter)                                               \
        _f(__VA_ARGS__, _iter);                                               \
} while(0)

#define VECTOR_FIND_IF(_i, _t, _x, _f)  do {                                  \
    _t *_iter = VECTOR_BEGIN((_x));                                           \
    _t *_end = VECTOR_END((_x));                                              \
    for(;_iter != _end;++_iter)                                               \
    {                                                                         \
        if(_f(_iter))                                                         \
            break;                                                            \
    }                                                                         \
    (_i) = _iter;                                                             \
} while(0)

#define VECTOR_FIND_IF_PARMS(_i, _t, _x, _f, ...)  do {                       \
    _t *_iter = VECTOR_BEGIN((_x));                                           \
    _t *_end = VECTOR_END((_x));                                              \
    for(;_iter != _end;++_iter)                                               \
    {                                                                         \
        if(_f(__VA_ARGS__, _iter))                                            \
            break;                                                            \
    }                                                                         \
    (_i) = _iter;                                                             \
} while(0)

#endif /* AL_VECTOR_H */
