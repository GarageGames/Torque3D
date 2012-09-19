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

#ifndef _TALGORITHM_H_
#define _TALGORITHM_H_


/// Finds the first matching value within the container
/// returning the the element or last if its not found.
template <class Iterator, class Value>
Iterator find(Iterator first, Iterator last, Value value)
{
   while (first != last && *first != value)
      ++first;
   return first;
}

/// Exchanges the values of the two elements.
template <typename T> 
inline void swap( T &left, T &right )
{
   T temp = right;
   right = left;
   left = temp;
}

/// Steps thru the elements of an array calling detete for each.
template <class Iterator, class Functor>
void for_each( Iterator first, Iterator last, Functor func )
{
   for ( ; first != last; first++ )
      func( *first );
}

/// Functor for deleting a pointer.
/// @see for_each
struct delete_pointer
{
  template <typename T>
  void operator()(T *ptr){ delete ptr;}
};

#endif //_TALGORITHM_H_
