#ifndef POLYMORPHISM_H
#define POLYMORPHISM_H

/* Macros to declare inheriting types, and to (down-)cast and up-cast. */
#define DERIVE_FROM_TYPE(t)          t t##_parent
#define STATIC_CAST(to, obj)         (&(obj)->to##_parent)
#ifdef __GNUC__
#define STATIC_UPCAST(to, from, obj) __extension__({                          \
    static_assert(__builtin_types_compatible_p(from, __typeof(*(obj))),       \
                  "Invalid upcast object from type");                         \
    (to*)((char*)(obj) - offsetof(to, from##_parent));                        \
})
#else
#define STATIC_UPCAST(to, from, obj) ((to*)((char*)(obj) - offsetof(to, from##_parent)))
#endif

/* Defines method forwards, which call the given parent's (T2's) implementation. */
#define DECLARE_FORWARD(T1, T2, rettype, func)                                \
rettype T1##_##func(T1 *obj)                                                  \
{ return T2##_##func(STATIC_CAST(T2, obj)); }

#define DECLARE_FORWARD1(T1, T2, rettype, func, argtype1)                     \
rettype T1##_##func(T1 *obj, argtype1 a)                                      \
{ return T2##_##func(STATIC_CAST(T2, obj), a); }

#define DECLARE_FORWARD2(T1, T2, rettype, func, argtype1, argtype2)           \
rettype T1##_##func(T1 *obj, argtype1 a, argtype2 b)                          \
{ return T2##_##func(STATIC_CAST(T2, obj), a, b); }

#define DECLARE_FORWARD3(T1, T2, rettype, func, argtype1, argtype2, argtype3) \
rettype T1##_##func(T1 *obj, argtype1 a, argtype2 b, argtype3 c)              \
{ return T2##_##func(STATIC_CAST(T2, obj), a, b, c); }

/* Defines method thunks, functions that call to the child's method. */
#define DECLARE_THUNK(T1, T2, rettype, func)                                  \
static rettype T1##_##T2##_##func(T2 *obj)                                    \
{ return T1##_##func(STATIC_UPCAST(T1, T2, obj)); }

#define DECLARE_THUNK1(T1, T2, rettype, func, argtype1)                       \
static rettype T1##_##T2##_##func(T2 *obj, argtype1 a)                        \
{ return T1##_##func(STATIC_UPCAST(T1, T2, obj), a); }

#define DECLARE_THUNK2(T1, T2, rettype, func, argtype1, argtype2)             \
static rettype T1##_##T2##_##func(T2 *obj, argtype1 a, argtype2 b)            \
{ return T1##_##func(STATIC_UPCAST(T1, T2, obj), a, b); }

#define DECLARE_THUNK3(T1, T2, rettype, func, argtype1, argtype2, argtype3)   \
static rettype T1##_##T2##_##func(T2 *obj, argtype1 a, argtype2 b, argtype3 c) \
{ return T1##_##func(STATIC_UPCAST(T1, T2, obj), a, b, c); }

#define DECLARE_THUNK4(T1, T2, rettype, func, argtype1, argtype2, argtype3, argtype4) \
static rettype T1##_##T2##_##func(T2 *obj, argtype1 a, argtype2 b, argtype3 c, argtype4 d) \
{ return T1##_##func(STATIC_UPCAST(T1, T2, obj), a, b, c, d); }

/* Defines the default functions used to (de)allocate a polymorphic object. */
#define DECLARE_DEFAULT_ALLOCATORS(T)                                         \
static void* T##_New(size_t size) { return al_malloc(16, size); }             \
static void T##_Delete(void *ptr) { al_free(ptr); }


/* Helper to extract an argument list for virtual method calls. */
#define EXTRACT_VCALL_ARGS(...)  __VA_ARGS__))

/* Call a "virtual" method on an object, with arguments. */
#define V(obj, func)  ((obj)->vtbl->func((obj), EXTRACT_VCALL_ARGS
/* Call a "virtual" method on an object, with no arguments. */
#define V0(obj, func) ((obj)->vtbl->func((obj) EXTRACT_VCALL_ARGS


/* Helper to extract an argument list for NEW_OBJ calls. */
#define EXTRACT_NEW_ARGS(...)  __VA_ARGS__);                                  \
    }                                                                         \
} while(0)

/* Allocate and construct an object, with arguments. */
#define NEW_OBJ(_res, T) do {                                                 \
    _res = T##_New(sizeof(T));                                                \
    if(_res)                                                                  \
    {                                                                         \
        memset(_res, 0, sizeof(T));                                           \
        T##_Construct(_res, EXTRACT_NEW_ARGS
/* Allocate and construct an object, with no arguments. */
#define NEW_OBJ0(_res, T) do {                                                \
    _res = T##_New(sizeof(T));                                                \
    if(_res)                                                                  \
    {                                                                         \
        memset(_res, 0, sizeof(T));                                           \
        T##_Construct(_res EXTRACT_NEW_ARGS

/* Destructs and deallocate an object. */
#define DELETE_OBJ(obj) do {                                                  \
    if((obj) != NULL)                                                         \
    {                                                                         \
        V0((obj),Destruct)();                                                 \
        V0((obj),Delete)();                                                   \
    }                                                                         \
} while(0)


/* Helper to get a type's vtable thunk for a child type. */
#define GET_VTABLE2(T1, T2) (&(T1##_##T2##_vtable))
/* Helper to set an object's vtable thunk for a child type. Used when constructing an object. */
#define SET_VTABLE2(T1, T2, obj) (STATIC_CAST(T2, obj)->vtbl = GET_VTABLE2(T1, T2))

#endif /* POLYMORPHISM_H */
