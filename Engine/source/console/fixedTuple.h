//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _FIXEDTUPLE_H_
#define _FIXEDTUPLE_H_
#include "engineTypes.h"

template <size_t I, typename ...Ts>
struct StdTupleIndexedHelper
{
   static const int getSize(std::tuple<Ts...>& tuple)
   {
      int size = sizeof(std::tuple_element<I, std::tuple<Ts>>(tuple));
      if (I != 0)
      {
         size += StdTupleIndexedHelper<I - 1, Ts...>::getSize();
      }
      return size;
   }

   static const int getOffset(std::tuple<Ts...>& tuple)
   {
      return (int)((size_t)&std::get<I>(tuple)) - ((size_t)&tuple);
   }
};


template <size_t I, typename T, typename ...TailTs>
struct BlockTupleIndexedHelper
{
	static const int getSize()
	{
		return sizeof(T) + BlockTupleIndexedHelper<I-1, TailTs...>::getSize();
	}

	static const int getOffset()
	{
		return sizeof(T) + BlockTupleIndexedHelper<I-1, TailTs...>::getOffset();
	}

	using type = T;
};

template <typename T, typename ...TailTs>
struct BlockTupleIndexedHelper<0, T, TailTs...>
{
	static const int getSize()
	{
		return sizeof(T);
	}

	static const int getOffset()
	{
		return 0;
	}

	using type = T;
};

template <typename T, typename ...TailTs>
struct BlockTupleHelper
{
	const static int getSize()
	{
		return sizeof(T) + BlockTupleHelper<TailTs...>::getSize();
	}

	const static size_t size = sizeof(T) + BlockTupleHelper<TailTs...>::size;
};

template<typename T>
struct BlockTupleHelper<T>
{
	const static int getSize()
	{
		return sizeof(T);
	}

	const static size_t size = sizeof(T);
};

template< typename T >
struct BlockTuple
{
};

template<typename R, typename ...ArgTs>
struct BlockTuple< R(ArgTs...) >
{
	char data[BlockTupleHelper<ArgTs...>::size];
private:
	using SelfType = BlockTuple< void(ArgTs...) >;

	template<size_t ...> struct Seq {};
	template<size_t N, size_t ...S> struct Gens : Gens<N - 1, N - 1, S...> {};

	template<size_t ...I> struct Gens<0, I...> { typedef Seq<I...> type; };

	using SeqType = typename Gens<sizeof...(ArgTs)>::type;

public:

	template<size_t I>
	static typename BlockTupleIndexedHelper<I, ArgTs...>::type& get(BlockTuple<R(ArgTs ...)>& tuple)
	{
		return *reinterpret_cast<typename BlockTupleIndexedHelper<I, ArgTs...>::type*>(
			((size_t)& tuple.data)
			+ BlockTupleIndexedHelper<I, ArgTs...>::getOffset()
		);
	}

	template<size_t I, typename HeadT, typename  ...TailTs>
	static typename BlockTupleIndexedHelper<I, HeadT, TailTs...>::type& set(BlockTuple<R(ArgTs ...)>& tuple, HeadT head, TailTs... tail)
	{
		return *reinterpret_cast<typename BlockTupleIndexedHelper<I, ArgTs...>::type*>(
			((size_t)& tuple.data)
			+ BlockTupleIndexedHelper<I, ArgTs...>::getOffset()
		);
	}

	BlockTuple(char data[BlockTupleHelper<ArgTs...>::size])
	{
		dMemmove(this->data, data, BlockTupleHelper<ArgTs...>::size);
	}
};

template <typename ...Ts>
struct InheritanceTuple;

template< typename T, typename ...TailTs >
struct InheritanceTuple<T, TailTs...> : InheritanceTuple<TailTs...>
{
	T data;

   using SelfType = InheritanceTuple<T, TailTs...>;
   using SuperType = InheritanceTuple<TailTs...>;

   template<size_t I, typename ...Ts>
   typename std::tuple_element<I, std::tuple<Ts...> >::type& get(InheritanceTuple<Ts...>& tuple)
   {
      if (I == 0)
      {
         return data;
      }
      return SuperType::get<I - 1>();
   }

   template<size_t I, typename ...Ts>
   typename std::tuple_element<I, std::tuple<Ts...> >::type& get(const InheritanceTuple<Ts...>& tuple)
   {
      if (I == 0)
      {
         return data;
      }
      return SuperType::get<I - 1>();
   }
};

template< typename T >
struct InheritanceTuple<T>
{
	T data;

   template<size_t I, typename ...Ts>
   typename std::tuple_element<I, std::tuple<Ts...> >::type& get(InheritanceTuple<Ts...>& tuple)
   {
      return data;
   }

   template<size_t I, typename ...Ts>
   typename std::tuple_element<I, std::tuple<Ts...> >::type& get(const InheritanceTuple<Ts...>& tuple)
   {
      return data;
   }
};

template <>
struct InheritanceTuple<> {};

template< typename T >
struct _InheritanceTuple {};

template< typename R, typename ...ArgTs >
struct _InheritanceTuple<R(ArgTs...)> : InheritanceTuple<ArgTs...> {};

template < ::std::size_t i>
struct InheritanceTupleAccessor
{
   template <class... Ts>
   static inline typename std::tuple_element<i, std::tuple<Ts...> >::type& get(InheritanceTuple<Ts...>& t)
   {
      return InheritanceTupleAccessor<i - 1>::get(t.rest);
   }
};

template <>
struct InheritanceTupleAccessor<0>
{
   template <class... Ts>
   static inline typename std::tuple_element<0, std::tuple<Ts...> >::type& get(InheritanceTuple<Ts...>& t)
   {
      return t.data;
   }
};

/*
template<typename R, typename ...ArgTs>
struct InheritanceTuple< R(ArgTs...) >
{
	char data[BlockTupleHelper<ArgTs...>::size];
private:
	using SelfType = BlockTuple< void(ArgTs...) >;

	template<size_t ...> struct Seq {};
	template<size_t N, size_t ...S> struct Gens : Gens<N - 1, N - 1, S...> {};

	template<size_t ...I> struct Gens<0, I...> { typedef Seq<I...> type; };

	using SeqType = typename Gens<sizeof...(ArgTs)>::type;

	//typename EngineTypeTraits<Type>::ArgumentToValue(StoreType);
public:

	template<size_t I>
	static typename BlockTupleIndexedHelper<I, ArgTs...>::type& get(BlockTuple<R(ArgTs ...)>& tuple)
	{
		return *reinterpret_cast<typename BlockTupleIndexedHelper<I, ArgTs...>::type*>(
			((size_t)& tuple.data)
			+ BlockTupleIndexedHelper<I, ArgTs...>::getOffset()
		);
	}

	BlockTuple(char data[BlockTupleHelper<ArgTs...>::size])
	{
		dMemmove(this->data, data, BlockTupleHelper<ArgTs...>::size);
	}
};
*/

/// @name Fixed-layout tuple definition
/// These structs and templates serve as a way to pass arguments from external 
/// applications and into the T3D console system.
/// They work as std::tuple, but they ensure a standardized fixed memory 
/// layout. Allowing for unmanaged calls with these tuples as the parameter 
/// lists.
///
/// The implementation is from a SO solution:
/// https://codereview.stackexchange.com/a/52279
/// As out use-case is pretty simple, this code could probably be simplified by 
/// stripping out a lot of extra functionality. But eh.
///
/// @{

template <typename ...Ts>
struct fixed_tuple;

template <typename T, typename ...Ts>
struct fixed_tuple<T, Ts...>
{
   T first;
   fixed_tuple<Ts...> rest;

   fixed_tuple() = default;
   template <class U, class...Us, class = typename ::std::enable_if<!::std::is_base_of<fixed_tuple, typename ::std::decay<U>::type>::value>::type>
   fixed_tuple(U&& u, Us&&...tail) :
      first(::std::forward<U>(u)),
      rest(::std::forward<Us>(tail)...) {}


};

template <typename T>
struct fixed_tuple<T>
{
   T first;

   fixed_tuple() = default;
   template <class U, class = typename ::std::enable_if<!::std::is_base_of<fixed_tuple, typename ::std::decay<U>::type>::value>::type>
   fixed_tuple(U&& u) :
      first(::std::forward<U>(u)) {}
};

template <>
struct fixed_tuple<> {};


template < ::std::size_t i, class T>
struct fixed_tuple_element;

template < ::std::size_t i, class T, class... Ts>
struct fixed_tuple_element<i, fixed_tuple<T, Ts...> >
   : fixed_tuple_element<i - 1, fixed_tuple<Ts...> >
{};

template <class T, class... Ts>
struct fixed_tuple_element<0, fixed_tuple<T, Ts...> >
{
   using type = T;
};

template < ::std::size_t i>
struct fixed_tuple_accessor
{
   template <class... Ts>
   static inline typename fixed_tuple_element<i, fixed_tuple<Ts...> >::type & get(fixed_tuple<Ts...> & t)
   {
      return fixed_tuple_accessor<i - 1>::get(t.rest);
   }

   template <class... Ts>
   static inline const typename fixed_tuple_element<i, fixed_tuple<Ts...> >::type & get(const fixed_tuple<Ts...> & t)
   {
      return fixed_tuple_accessor<i - 1>::get(t.rest);
   }

   template <class... Ts>
   static inline typename fixed_tuple_element<i, fixed_tuple<Ts...> >::type * getRef(fixed_tuple<Ts...> & t)
   {
      return fixed_tuple_accessor<i - 1>::getRef(t.rest);
   }

   template <class... Ts>
   static inline const typename fixed_tuple_element<i, fixed_tuple<Ts...> >::type * getRef(const fixed_tuple<Ts...> & t)
   {
      return fixed_tuple_accessor<i - 1>::getRef(t.rest);
   }

   template <class... Ts>
   static inline U32 getOffset(fixed_tuple<Ts...> & t)
   {
      return (U32)((size_t)fixed_tuple_accessor<i>::getRef(t)) - ((size_t)& t);
   }

   template <class... Ts>
   static inline const U32 getOffset(const fixed_tuple<Ts...> & t)
   {
      return (U32)((size_t)fixed_tuple_accessor<i>::getRef(t)) - ((size_t)& t);
   }
};

template <>
struct fixed_tuple_accessor<0>
{
   template <class... Ts>
   static inline typename fixed_tuple_element<0, fixed_tuple<Ts...> >::type & get(fixed_tuple<Ts...> & t)
   {
      return t.first;
   }

   template <class... Ts>
   static inline const typename fixed_tuple_element<0, fixed_tuple<Ts...> >::type & get(const fixed_tuple<Ts...> & t)
   {
      return t.first;
   }
   template <class... Ts>
   static inline typename fixed_tuple_element<0, fixed_tuple<Ts...> >::type * getRef(fixed_tuple<Ts...> & t)
   {
      return &t.first;
   }

   template <class... Ts>
   static inline const typename fixed_tuple_element<0, fixed_tuple<Ts...> >::type * getRef(const fixed_tuple<Ts...> & t)
   {
      return &t.first;
   }
   template <class... Ts>
   static inline U32 getOffset(fixed_tuple<Ts...> & t)
   {
      return (int)((size_t)& t.first) - ((size_t)& t);
   }

   template <class... Ts>
   static inline const U32 getOffset(const fixed_tuple<Ts...> & t)
   {
      return (int)((size_t)& t.first) - ((size_t)& t);
   }
};

template< typename T1, typename T2 >
struct fixed_tuple_mutator {};

template<typename... Tsrc, typename... Tdest>
struct fixed_tuple_mutator<void(Tsrc...), void(Tdest...)>
{
   template<std::size_t I = 0>
   static inline typename std::enable_if<I == sizeof...(Tsrc), void>::type
      copy_r_t_l(fixed_tuple<Tsrc...>& src, fixed_tuple<Tdest...>& dest)
   { }

   template<std::size_t I = 0>
   static inline typename std::enable_if<I < sizeof...(Tsrc), void>::type
      copy_r_t_l(fixed_tuple<Tsrc...>& src, fixed_tuple<Tdest...>& dest)
   {
      fixed_tuple_accessor<I + (sizeof...(Tdest)-sizeof...(Tsrc))>::get(dest) = fixed_tuple_accessor<I>::get(src);
      copy_r_t_l<I + 1>(src, dest);
   }

   template<std::size_t I = 0>
   static inline typename std::enable_if<I == sizeof...(Tsrc), void>::type
      copy(std::tuple<Tsrc...>& src, fixed_tuple<Tdest...>& dest)
   { }

   template<std::size_t I = 0>
   static inline typename std::enable_if<I < sizeof...(Tsrc), void>::type
      copy(std::tuple<Tsrc...>& src, fixed_tuple<Tdest...>& dest)
   {
      fixed_tuple_accessor<I>::get(dest) = std::get<I>(src);
      copy<I + 1>(src, dest);
   }

   template<std::size_t I = 0>
   static inline typename std::enable_if<I == sizeof...(Tsrc), void>::type
      copyPtrs(std::tuple<Tsrc...>& src, fixed_tuple<Tdest...>& dest)
   { }

   template<std::size_t I = 0>
   static inline typename std::enable_if<I < sizeof...(Tsrc), void>::type
      copyPtrs(std::tuple<Tsrc...>& src, fixed_tuple<Tdest...>& dest)
   {
      fixed_tuple_accessor<I>::get(dest) = &std::get<I>(src);
      copyPtrs<I + 1>(src, dest);
   }
};


/// @}


#endif // !_FIXEDTUPLE_H_
