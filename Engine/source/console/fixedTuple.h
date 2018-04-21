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
};

template< typename T1, typename T2 >
struct fixed_tuple_mutator {};

template<typename... Tdest, typename... Tsrc>
struct fixed_tuple_mutator<void(Tdest...), void(Tsrc...)>
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
};


/// @}


#endif // !_FIXEDTUPLE_H_