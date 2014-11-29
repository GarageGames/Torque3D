// File: crn_traits.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   template<typename T>
   struct scalar_type
   {
      enum { cFlag = false };
      static inline void construct(T* p) { helpers::construct(p); }
      static inline void construct(T* p, const T& init) { helpers::construct(p, init); }
      static inline void construct_array(T* p, uint n) { helpers::construct_array(p, n); }
      static inline void destruct(T* p) { helpers::destruct(p); }
      static inline void destruct_array(T* p, uint n) { helpers::destruct_array(p, n); }
   };

   template<typename T> struct scalar_type<T*>
   {
      enum { cFlag = true };
      static inline void construct(T** p) { memset(p, 0, sizeof(T*)); }
      static inline void construct(T** p, T* init) { *p = init; }
      static inline void construct_array(T** p, uint n) { memset(p, 0, sizeof(T*) * n); }
      static inline void destruct(T** p) { p; }
      static inline void destruct_array(T** p, uint n) { p, n; }
   };

#define CRNLIB_DEFINE_BUILT_IN_TYPE(X) \
   template<> struct scalar_type<X> { \
   enum { cFlag = true }; \
   static inline void construct(X* p) { memset(p, 0, sizeof(X)); } \
   static inline void construct(X* p, const X& init) { memcpy(p, &init, sizeof(X)); } \
   static inline void construct_array(X* p, uint n) { memset(p, 0, sizeof(X) * n); } \
   static inline void destruct(X* p) { p; } \
   static inline void destruct_array(X* p, uint n) { p, n; } };

   CRNLIB_DEFINE_BUILT_IN_TYPE(bool)
   CRNLIB_DEFINE_BUILT_IN_TYPE(char)
   CRNLIB_DEFINE_BUILT_IN_TYPE(unsigned char)
   CRNLIB_DEFINE_BUILT_IN_TYPE(short)
   CRNLIB_DEFINE_BUILT_IN_TYPE(unsigned short)
   CRNLIB_DEFINE_BUILT_IN_TYPE(int)
   CRNLIB_DEFINE_BUILT_IN_TYPE(unsigned int)
   CRNLIB_DEFINE_BUILT_IN_TYPE(long)
   CRNLIB_DEFINE_BUILT_IN_TYPE(unsigned long)
   CRNLIB_DEFINE_BUILT_IN_TYPE(__int64)
   CRNLIB_DEFINE_BUILT_IN_TYPE(unsigned __int64)
   CRNLIB_DEFINE_BUILT_IN_TYPE(float)
   CRNLIB_DEFINE_BUILT_IN_TYPE(double)
   CRNLIB_DEFINE_BUILT_IN_TYPE(long double)

#undef CRNLIB_DEFINE_BUILT_IN_TYPE

// See: http://erdani.org/publications/cuj-2004-06.pdf

   template<typename T>
   struct bitwise_movable { enum { cFlag = false }; };

// Defines type Q as bitwise movable.
#define CRNLIB_DEFINE_BITWISE_MOVABLE(Q) template<> struct bitwise_movable<Q> { enum { cFlag = true }; };

   template<typename T>
   struct bitwise_copyable { enum { cFlag = false }; };

   // Defines type Q as bitwise copyable.
#define CRNLIB_DEFINE_BITWISE_COPYABLE(Q) template<> struct bitwise_copyable<Q> { enum { cFlag = true }; };

#define CRNLIB_IS_POD(T) __is_pod(T)

#define CRNLIB_IS_SCALAR_TYPE(T) (scalar_type<T>::cFlag)

#define CRNLIB_IS_BITWISE_COPYABLE(T) ((scalar_type<T>::cFlag) || (bitwise_copyable<T>::cFlag) || CRNLIB_IS_POD(T))

#define CRNLIB_IS_BITWISE_MOVABLE(T) (CRNLIB_IS_BITWISE_COPYABLE(T) || (bitwise_movable<T>::cFlag))

#define CRNLIB_HAS_DESTRUCTOR(T) ((!scalar_type<T>::cFlag) && (!__is_pod(T)))

   // From yasli_traits.h:
   // Credit goes to Boost;
   // also found in the C++ Templates book by Vandevoorde and Josuttis

   typedef char (&yes_t)[1];
   typedef char (&no_t)[2];

   template <class U> yes_t class_test(int U::*);
   template <class U> no_t class_test(...);

   template <class T> struct is_class
   {
      enum { value = (sizeof(class_test<T>(0)) == sizeof(yes_t)) };
   };

   template <typename T> struct is_pointer
   {
      enum { value = false };
   };

   template <typename T> struct is_pointer<T*>
   {
      enum { value = true };
   };

   CRNLIB_DEFINE_BITWISE_COPYABLE(empty_type);
   CRNLIB_DEFINE_BITWISE_MOVABLE(empty_type);

} // namespace crnlib
