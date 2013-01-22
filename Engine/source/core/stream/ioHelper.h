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

#ifndef _UTIL_IOHELPER_H_
#define _UTIL_IOHELPER_H_

#ifndef _CORE_STREAM_H_
#include "core/stream/stream.h"
#endif

/// Helper templates to aggregate IO operations - generally used in
/// template expansion.
namespace IOHelper
{
   template<class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K, class L, class M>
      inline bool reads(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h,I& i,J& j,K& k,L& l,M& m)
   { s->read(&a); s->read(&b); s->read(&c); s->read(&d); s->read(&e); s->read(&f); s->read(&g); s->read(&h); s->read(&i); s->read(&j); s->read(&k); s->read(&l); return s->read(&m); }

   template<class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K, class L>
      inline bool reads(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h,I& i,J& j,K& k,L& l)
   { s->read(&a); s->read(&b); s->read(&c); s->read(&d); s->read(&e); s->read(&f); s->read(&g); s->read(&h); s->read(&i); s->read(&j); s->read(&k); return s->read(&l); }

   template<class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K>
      inline bool reads(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h,I& i,J& j,K& k)
   { s->read(&a); s->read(&b); s->read(&c); s->read(&d); s->read(&e); s->read(&f); s->read(&g); s->read(&h); s->read(&i); s->read(&j); return s->read(&k); }

   template<class A,class B,class C,class D,class E,class F,class G,class H,class I,class J>
      inline bool reads(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h,I& i,J& j)
   { s->read(&a); s->read(&b); s->read(&c); s->read(&d); s->read(&e); s->read(&f); s->read(&g); s->read(&h); s->read(&i); return s->read(&j); }

   template<class A,class B,class C,class D,class E,class F,class G,class H,class I>
      inline bool reads(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h,I& i)
   { s->read(&a); s->read(&b); s->read(&c); s->read(&d); s->read(&e); s->read(&f); s->read(&g); s->read(&h); return s->read(&i); }

   template<class A,class B,class C,class D,class E,class F,class G,class H>
      inline bool reads(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h)
   { s->read(&a); s->read(&b); s->read(&c); s->read(&d); s->read(&e); s->read(&f); s->read(&g); return s->read(&h); }

   template<class A,class B,class C,class D,class E,class F,class G>
      inline bool reads(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g)
   { s->read(&a); s->read(&b); s->read(&c); s->read(&d); s->read(&e); s->read(&f); return s->read(&g); }

   template<class A,class B,class C,class D,class E,class F>
      inline bool reads(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f)
   { s->read(&a); s->read(&b); s->read(&c); s->read(&d); s->read(&e); return s->read(&f); }

   template<class A,class B,class C,class D,class E>
      inline bool reads(Stream *s,A& a,B& b,C& c,D& d,E& e)
   { s->read(&a); s->read(&b); s->read(&c); s->read(&d); return s->read(&e); }

   template<class A,class B,class C,class D>
      inline bool reads(Stream *s,A& a,B& b,C& c,D& d)
   { s->read(&a); s->read(&b); s->read(&c); return s->read(&d); }

   template<class A,class B,class C>
      inline bool reads(Stream *s,A& a,B& b,C& c)
   { s->read(&a); s->read(&b); return s->read(&c); }

   template<class A,class B>
      inline bool reads(Stream *s,A& a,B& b)
   { s->read(&a); return s->read(&b); }

   template<class A>
      inline bool reads(Stream *s,A& a)
   { return s->read(&a); }

   template<class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K,class L,class M>
      inline bool writes(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h,I& i,J& j,K& k,L& l,M& m)
   { s->write(a); s->write(b); s->write(c); s->write(d); s->write(e); s->write(f); s->write(g); s->write(h); s->write(i); s->write(j); s->write(k); s->write(l); return s->write(m); }

   template<class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K,class L>
      inline bool writes(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h,I& i,J& j,K& k,L& l)
   { s->write(a); s->write(b); s->write(c); s->write(d); s->write(e); s->write(f); s->write(g); s->write(h); s->write(i); s->write(j); s->write(k); return s->write(l); }

   template<class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K>
      inline bool writes(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h,I& i,J& j,K& k)
   { s->write(a); s->write(b); s->write(c); s->write(d); s->write(e); s->write(f); s->write(g); s->write(h); s->write(i); s->write(j); return s->write(k); }

   template<class A,class B,class C,class D,class E,class F,class G,class H,class I,class J>
      inline bool writes(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h,I& i,J& j)
   { s->write(a); s->write(b); s->write(c); s->write(d); s->write(e); s->write(f); s->write(g); s->write(h); s->write(i); return s->write(j); }

   template<class A,class B,class C,class D,class E,class F,class G,class H,class I>
      inline bool writes(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h,I& i)
   { s->write(a); s->write(b); s->write(c); s->write(d); s->write(e); s->write(f); s->write(g); s->write(h); return s->write(i); }

   template<class A,class B,class C,class D,class E,class F,class G,class H>
      inline bool writes(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g,H& h)
   { s->write(a); s->write(b); s->write(c); s->write(d); s->write(e); s->write(f); s->write(g); return s->write(h); }

   template<class A,class B,class C,class D,class E,class F,class G>
      inline bool writes(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f,G& g)
   { s->write(a); s->write(b); s->write(c); s->write(d); s->write(e); s->write(f); return s->write(g); }

   template<class A,class B,class C,class D,class E,class F>
      inline bool writes(Stream *s,A& a,B& b,C& c,D& d,E& e,F& f)
   { s->write(a); s->write(b); s->write(c); s->write(d); s->write(e); return s->write(f); }

   template<class A,class B,class C,class D,class E>
      inline bool writes(Stream *s,A& a,B& b,C& c,D& d,E& e)
   { s->write(a); s->write(b); s->write(c); s->write(d); return s->write(e); }

   template<class A,class B,class C,class D>
      inline bool writes(Stream *s,A& a,B& b,C& c,D& d)
   { s->write(a); s->write(b); s->write(c); return s->write(d); }

   template<class A,class B,class C>
      inline bool writes(Stream *s,A& a,B& b,C& c)
   { s->write(a); s->write(b); return s->write(c); }

   template<class A,class B>
      inline bool writes(Stream *s,A& a,B& b)
   { s->write(a); return s->write(b); }

   template<class A>
      inline bool writes(Stream *s,A& a)
   { return s->write(a); }
}

#endif