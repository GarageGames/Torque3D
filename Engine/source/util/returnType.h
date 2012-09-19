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

#ifndef _UTIL_RETURNTYPE_H_
#define _UTIL_RETURNTYPE_H_

/// @file
///
/// Helper templates to determine the return type of functions.

template <class R> struct ReturnType { typedef void ValueType; };

template <class R,class A,class B,class C,class D,class E,class F,class G>
struct ReturnType<R (*)(A,B,C,D,E,F,G)> { typedef R ValueType; };
template <class R,class A,class B,class C,class D,class E,class F>
struct ReturnType<R (*)(A,B,C,D,E,F)> { typedef R ValueType; };
template <class R,class A,class B,class C,class D,class E>
struct ReturnType<R (*)(A,B,C,D,E)> { typedef R ValueType; };
template <class R,class A,class B,class C,class D>
struct ReturnType<R (*)(A,B,C,D)> { typedef R ValueType; };
template <class R,class A,class B,class C>
struct ReturnType<R (*)(A,B,C)> { typedef R ValueType; };
template <class R,class A,class B>
struct ReturnType<R (*)(A,B)> { typedef R ValueType; };
template <class R,class A>
struct ReturnType<R (*)(A)> { typedef R ValueType; };
template <class R>
struct ReturnType<R (*)()> { typedef R ValueType; };

template <class R,class O,class A,class B,class C,class D,class E,class F,class G>
struct ReturnType<R (O::*)(A,B,C,D,E,F,G)> { typedef R ValueType; };
template <class R,class O,class A,class B,class C,class D,class E,class F>
struct ReturnType<R (O::*)(A,B,C,D,E,F)> { typedef R ValueType; };
template <class R,class O,class A,class B,class C,class D,class E>
struct ReturnType<R (O::*)(A,B,C,D,E)> { typedef R ValueType; };
template <class R,class O,class A,class B,class C,class D>
struct ReturnType<R (O::*)(A,B,C,D)> { typedef R ValueType; };
template <class R,class O,class A,class B,class C>
struct ReturnType<R (O::*)(A,B,C)> { typedef R ValueType; };
template <class R,class O,class A,class B>
struct ReturnType<R (O::*)(A,B)> { typedef R ValueType; };
template <class R,class O,class A>
struct ReturnType<R (O::*)(A)> { typedef R ValueType; };
template <class R,class O>
struct ReturnType<R (O::*)()> { typedef R ValueType; };

#endif