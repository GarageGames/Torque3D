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

#ifndef _UTIL_JOURNAL_JOURNAL_H_
#define _UTIL_JOURNAL_JOURNAL_H_ 

#include "platform/platform.h"
#include "core/stream/stream.h"
#include "util/returnType.h"
#include "core/stream/ioHelper.h"
#include "core/idGenerator.h"

/// Journaling System.
///
/// The journaling system is used to record external events and
/// non-deterministic values from an execution run in order that the
/// run may be re-played for debugging purposes.  The journaling
/// system is integrated into the platform library, though not all
/// platform calls are journaled.
///
/// File system calls are not journaled, so any modified files must
/// be reset to their original state before playback.  Only a single
/// journal can be recored or played back at a time.
///
/// For the journals to play back correctly, journal events cannot
/// be triggered during the processing of another event.
class Journal 
{
   Journal() {}
   ~Journal();

   static Journal smInstance;

   typedef U32 Id;
   typedef void* VoidPtr;
   typedef void (Journal::*VoidMethod)();

   /// Functor base classes
   struct Functor 
   {
      Functor() {}
      virtual ~Functor() {}
      virtual void read(Stream *s) = 0;
      virtual void dispatch() = 0;
   };

   /// Multiple argument function functor specialization
   template <class T>
   struct FunctorDecl: public Functor  {
      typedef void(*FuncPtr)();
      FuncPtr ptr;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   {}
      void dispatch()                  { (*ptr)(); }
   };

   template <class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K,class L,class M>
   struct FunctorDecl< void(*)(A,B,C,D,E,F,G,H,I,J,K,L,M) >: public Functor {
      typedef void(*FuncPtr)(A,B,C,D,E,F,G,H,I,J,K,L,M);
      FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h; I i; J j; K k; L l; M m;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h,i,j,k,l,m); }
      void dispatch()                  { (*ptr)(a,b,c,d,e,f,g,h,i,j,k,l,m); }
   };

   template <class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K,class L>
   struct FunctorDecl< void(*)(A,B,C,D,E,F,G,H,I,J,K,L) >: public Functor {
      typedef void(*FuncPtr)(A,B,C,D,E,F,G,H,I,J,K,L);
      FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h; I i; J j; K k; L l;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h,i,j,k,l); }
      void dispatch()                  { (*ptr)(a,b,c,d,e,f,g,h,i,j,k,l); }
   };

   template <class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K>
   struct FunctorDecl< void(*)(A,B,C,D,E,F,G,H,I,J,K) >: public Functor {
      typedef void(*FuncPtr)(A,B,C,D,E,F,G,H,I,J,K);
      FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h; I i; J j; K k;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h,i,j,k); }
      void dispatch()                  { (*ptr)(a,b,c,d,e,f,g,h,i,j,k); }
   };

   template <class A,class B,class C,class D,class E,class F,class G,class H,class I,class J>
   struct FunctorDecl< void(*)(A,B,C,D,E,F,G,H,I,J) >: public Functor {
      typedef void(*FuncPtr)(A,B,C,D,E,F,G,H,I,J);
      FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h; I i; J j;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h,i,j); }
      void dispatch()                  { (*ptr)(a,b,c,d,e,f,g,h,i,j); }
   };

   template <class A,class B,class C,class D,class E,class F,class G,class H,class I>
   struct FunctorDecl< void(*)(A,B,C,D,E,F,G,H,I) >: public Functor {
      typedef void(*FuncPtr)(A,B,C,D,E,F,G,H,I);
      FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h; I i;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h,i); }
      void dispatch()                  { (*ptr)(a,b,c,d,e,f,g,h,i); }
   };

   template <class A,class B,class C,class D,class E,class F,class G,class H>
   struct FunctorDecl< void(*)(A,B,C,D,E,F,G,H) >: public Functor {
      typedef void(*FuncPtr)(A,B,C,D,E,F,G,H);
      FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g; H h;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h); }
      void dispatch()                  { (*ptr)(a,b,c,d,e,f,g,h); }
   };

   template <class A,class B,class C,class D,class E,class F,class G>
   struct FunctorDecl< void(*)(A,B,C,D,E,F,G) >: public Functor {
      typedef void(*FuncPtr)(A,B,C,D,E,F,G);
      FuncPtr ptr; A a; B b; C c; D d; E e; F f; G g;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g); }
      void dispatch()                  { (*ptr)(a,b,c,d,e,f,g); }
   };

   template <class A,class B,class C,class D,class E,class F>
   struct FunctorDecl< void(*)(A,B,C,D,E,F) >: public Functor {
      typedef void(*FuncPtr)(A,B,C,D,E,F);
      FuncPtr ptr; A a; B b; C c; D d; E e; F f;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f); }
      void dispatch()                  { (*ptr)(a,b,c,d,e,f); }
   };

   template <class A,class B,class C,class D,class E>
   struct FunctorDecl< void(*)(A,B,C,D,E) >: public Functor {
      typedef void(*FuncPtr)(A,B,C,D,E);
      FuncPtr ptr; A a; B b; C c; D d; E e;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e); }
      void dispatch()                  { (*ptr)(a,b,c,d,e); }
   };

   template <class A,class B,class C,class D>
   struct FunctorDecl< void(*)(A,B,C,D) >: public Functor {
      typedef void(*FuncPtr)(A,B,C,D);
      FuncPtr ptr; A a; B b; C c; D d;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d); }
      void dispatch()                  { (*ptr)(a,b,c,d); }
   };

   template <class A,class B,class C>
   struct FunctorDecl< void(*)(A,B,C) >: public Functor {
      typedef void(*FuncPtr)(A,B,C);
      FuncPtr ptr; A a; B b; C c;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c); }
      void dispatch()                  { (*ptr)(a,b,c); }
   };

   template <class A,class B>
   struct FunctorDecl< void(*)(A,B) >: public Functor {
      typedef void(*FuncPtr)(A,B);
      FuncPtr ptr; A a; B b;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b); }
      void dispatch()                  { (*ptr)(a,b); }
   };

   template <class A>
   struct FunctorDecl< void(*)(A) >: public Functor {
      typedef void(*FuncPtr)(A);
      FuncPtr ptr; A a;
      FunctorDecl(FuncPtr p): ptr(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a); }
      void dispatch()                  { (*ptr)(a); }
   };

   // Multiple argument object member function functor specialization
   template <class T,class U>
   struct MethodDecl: public Functor  {
      typedef T ObjPtr;
      typedef U MethodPtr;
      ObjPtr obj; MethodPtr method;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   {}
      void dispatch()                  { (obj->*method)(); }
   };

   template <class T,class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K,class L,class M>
   struct MethodDecl<T*, void(T::*)(A,B,C,D,E,F,G,H,I,J,K,L,M) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C,D,E,F,G,H,I,J,K,L,M);
      ObjPtr obj; MethodPtr method; A a; B b; C c; D d; E e; F f; G g; H h; I i; J j; K k; L l; M m;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h,i,j,k,l,m); }
      void dispatch()                  { (obj->*method)(a,b,c,d,e,f,g,h,i,j,k,l,m); }
   };

   template <class T,class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K,class L>
   struct MethodDecl<T*, void(T::*)(A,B,C,D,E,F,G,H,I,J,K,L) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C,D,E,F,G,H,I,J,K,L);
      ObjPtr obj; MethodPtr method; A a; B b; C c; D d; E e; F f; G g; H h; I i; J j; K k; L l;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h,i,j,k,l); }
      void dispatch()                  { (obj->*method)(a,b,c,d,e,f,g,h,i,j,k,l); }
   };

   template <class T,class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K>
   struct MethodDecl<T*, void(T::*)(A,B,C,D,E,F,G,H,I,J,K) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C,D,E,F,G,H,I,J,K);
      ObjPtr obj; MethodPtr method; A a; B b; C c; D d; E e; F f; G g; H h; I i; J j; K k;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h,i,j,k); }
      void dispatch()                  { (obj->*method)(a,b,c,d,e,f,g,h,i,j,k); }
   };

   template <class T,class A,class B,class C,class D,class E,class F,class G,class H,class I,class J>
   struct MethodDecl<T*, void(T::*)(A,B,C,D,E,F,G,H,I,J) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C,D,E,F,G,H,I,J);
      ObjPtr obj; MethodPtr method; A a; B b; C c; D d; E e; F f; G g; H h; I i; J j;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h,i,j); }
      void dispatch()                  { (obj->*method)(a,b,c,d,e,f,g,h,i,j); }
   };

   template <class T,class A,class B,class C,class D,class E,class F,class G,class H,class I>
   struct MethodDecl<T*, void(T::*)(A,B,C,D,E,F,G,H,I) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C,D,E,F,G,H,I);
      ObjPtr obj; MethodPtr method; A a; B b; C c; D d; E e; F f; G g; H h; I i;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h,i); }
      void dispatch()                  { (obj->*method)(a,b,c,d,e,f,g,h,i); }
   };

   template <class T,class A,class B,class C,class D,class E,class F,class G,class H>
   struct MethodDecl<T*, void(T::*)(A,B,C,D,E,F,G,H) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C,D,E,F,G,H);
      ObjPtr obj; MethodPtr method; A a; B b; C c; D d; E e; F f; G g; H h;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g,h); }
      void dispatch()                  { (obj->*method)(a,b,c,d,e,f,g,h); }
   };

   template <class T,class A,class B,class C,class D,class E,class F,class G>
   struct MethodDecl<T*, void(T::*)(A,B,C,D,E,F,G) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C,D,E,F,G);
      ObjPtr obj; MethodPtr method; A a; B b; C c; D d; E e; F f; G g;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f,g); }
      void dispatch()                  { (obj->*method)(a,b,c,d,e,f,g); }
   };

   template <class T,class A,class B,class C,class D,class E,class F>
   struct MethodDecl<T*, void(T::*)(A,B,C,D,E,F) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C,D,E,F);
      ObjPtr obj; MethodPtr method; A a; B b; C c; D d; E e; F f;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e,f); }
      void dispatch()                  { (obj->*method)(a,b,c,d,e,f); }
   };

   template <class T,class A,class B,class C,class D,class E>
   struct MethodDecl<T*, void(T::*)(A,B,C,D,E) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C,D,E);
      ObjPtr obj; MethodPtr method; A a; B b; C c; D d; E e;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d,e); }
      void dispatch()                  { (obj->*method)(a,b,c,d,e); }
   };

   template <class T,class A,class B,class C,class D>
   struct MethodDecl<T*, void(T::*)(A,B,C,D) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C,D);
      ObjPtr obj; MethodPtr method; A a; B b; C c; D d;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c,d); }
      void dispatch()                  { (obj->*method)(a,b,c,d); }
   };

   template <class T,class A,class B,class C>
   struct MethodDecl<T*, void(T::*)(A,B,C) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B,C);
      ObjPtr obj; MethodPtr method; A a; B b; C c;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b,c); }
      void dispatch()                  { (obj->*method)(a,b,c); }
   };

   template <class T,class A,class B>
   struct MethodDecl<T*, void(T::*)(A,B) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A,B);
      ObjPtr obj; MethodPtr method; A a; B b;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a,b); }
      void dispatch()                  { (obj->*method)(a,b); }
   };

   template <class T,class A>
   struct MethodDecl<T*, void(T::*)(A) >: public Functor {
      typedef T* ObjPtr;
      typedef void(T::*MethodPtr)(A);
      ObjPtr obj; MethodPtr method; A a;
      MethodDecl(ObjPtr o,MethodPtr p): obj(o), method(p)   {}
      void read(Stream *file)   { IOHelper::reads(file,a); }
      void dispatch()                  { (obj->*method)(a); }
   };

   // Function declarations
   struct FuncDecl {
      FuncDecl* next;
      Id id;
      virtual ~FuncDecl() {}
      virtual bool match(VoidPtr,VoidMethod) const = 0;
      virtual Functor* create() const = 0;
   };

   template<typename T>
   struct FuncRep: public FuncDecl {
      typename T::FuncPtr function;
      virtual bool match(VoidPtr ptr,VoidMethod) const {
         return function == (typename T::FuncPtr)ptr;
      }
      T* create() const { return new T(function); };
   };

   template<typename T>
   struct MethodRep: public FuncDecl {
      typename T::ObjPtr obj;
      typename T::MethodPtr method;
      virtual bool match(VoidPtr ptr,VoidMethod func) const {
         return obj == (typename T::ObjPtr)ptr && method == (typename T::MethodPtr)func;
      }
      T* create() const { return new T(obj,method); };
   };

   static FuncDecl* _FunctionList;

   static inline IdGenerator &idPool()
   {
      static IdGenerator _IdPool(1, 65535);
      return _IdPool;
   }

   static U32 _Count;
   static Stream *mFile;
   static enum Mode {
      StopState, PlayState, RecordState, DisabledState
   } _State;
   static bool _Dispatching;

   static Functor* _create(Id id);
   static void _start();
   static void _finish();
   static Id _getFunctionId(VoidPtr ptr,VoidMethod method);
   static void _removeFunctionId(VoidPtr ptr,VoidMethod method);

public:
   static void Record(const char * file);
   static void Play(const char * file);
   static bool PlayNext();
   static void Stop();
   static void Disable();

   /// Returns true if in recording mode.
   static inline bool IsRecording() {
      return _State == RecordState;
   }

   /// Returns true if in play mode.
   static inline bool IsPlaying() {
      return _State == PlayState;
   }

   /// Returns true if a function is being dispatched
   static inline bool IsDispatching() {
      return _Dispatching;
   }

   template<typename T>
   static void Read(T *v)
   {
      AssertFatal(IsPlaying(), "Journal::Read - not playing right now.");
      bool r = mFile->read(v);
      AssertFatal(r, "Journal::Read - failed to read!");
   }

   static bool Read(U32 size, void *buffer)
   {
      AssertFatal(IsPlaying(), "Journal::Read - not playing right now.");
      bool r = mFile->read(size, buffer);
      AssertFatal(r, "Journal::Read - failed to read!");
      return r;
   }

   static void ReadString(char str[256])
   {
      AssertFatal(IsPlaying(), "Journal::ReadString - not playing right now.");
      mFile->readString(str);
   }

   template<typename T>
   static void Write(const T &v)
   {
      AssertFatal(IsRecording(), "Journal::Write - not recording right now.");
      bool r = mFile->write(v);
      AssertFatal(r, "Journal::Write - failed to write!");
   }

   static bool Write(U32 size, void *buffer)
   {
      AssertFatal(IsRecording(), "Journal::Write - not recording right now.");
      bool r = mFile->write(size, buffer);
      AssertFatal(r, "Journal::Write - failed to write!");
      return r;   
   }

   static void WriteString(const char str[256])
   {
      AssertFatal(IsRecording(), "Journal::WriteString - not recording right now.");
      mFile->writeString(str);
   }

   /// Register a function with the journalling system.
   template<typename T>
   static void DeclareFunction(T func) {
      if (!_getFunctionId((VoidPtr)func,0)) {
         FuncRep<FunctorDecl<T> >* decl = new FuncRep<FunctorDecl<T> >;
         decl->function = func;
         decl->id = idPool().alloc();
         decl->next = _FunctionList;
         _FunctionList = decl;
      }
   }

   template<typename T, typename U>
   static void DeclareFunction(T obj, U method)
   {
      if (!_getFunctionId((VoidPtr)obj,(VoidMethod)method)) {
         MethodRep<MethodDecl<T,U> >* decl = new MethodRep<MethodDecl<T,U> >;
         decl->obj = obj;
         decl->method = method;
         decl->id = idPool().alloc();
         decl->next = _FunctionList;
         _FunctionList = decl;
      }
   }

   template<typename T, typename U>
   static void RemoveFunction(T obj, U method)
   {
      _removeFunctionId((VoidPtr)obj,(VoidMethod)method);
   }

   /// Journal a function's return value. The return value of the
   /// function is stored into the journal and retrieved during
   /// playback. During playback the function is not executed.
   #define Method(Func,Arg1,Arg2) \
      static typename ReturnType<Func>::ValueType Result Arg1 { \
         typename ReturnType<Func>::ValueType value; \
         if (_Dispatching) \
            return; \
         if (_State == PlayState) { \
            _start(); \
            IOHelper::reads(mFile,value); \
            _finish(); \
            return value; \
         } \
         _Dispatching = true; \
         value = (*func) Arg2; \
         _Dispatching = false; \
         if (_State == RecordState) { \
            _start(); \
            IOHelper::writes(mFile,value); \
            _finish(); \
         } \
         return value; \
      }

   template<class Func,class A,class B,class C,class D,class E,class F, class G, class H, class I, class J, class K, class L, class M>
      Method(Func,(Func func,A a,B b,C c,D d,E e,F f, G g, H h, I i, J j, K k, L l, M m),(a,b,c,d,e,f,g,h,i,j,k,l,m))
   template<class Func,class A,class B,class C,class D,class E,class F, class G, class H, class I, class J, class K, class L>
      Method(Func,(Func func,A a,B b,C c,D d,E e,F f, G g, H h, I i, J j, K k, L l),(a,b,c,d,e,f,g,h,i,j,k,l))
   template<class Func,class A,class B,class C,class D,class E,class F, class G, class H, class I, class J, class K>
      Method(Func,(Func func,A a,B b,C c,D d,E e,F f, G g, H h, I i, J j, K k),(a,b,c,d,e,f,g,h,i,j,k))
   template<class Func,class A,class B,class C,class D,class E,class F, class G, class H, class I, class J>
      Method(Func,(Func func,A a,B b,C c,D d,E e,F f, G g, H h, I i, J j),(a,b,c,d,e,f,g,h,i,j))
   template<class Func,class A,class B,class C,class D,class E,class F, class G, class H, class I>
      Method(Func,(Func func,A a,B b,C c,D d,E e,F f, G g, H h, I i),(a,b,c,d,e,f,g,h,i))
   template<class Func,class A,class B,class C,class D,class E,class F, class G, class H>
      Method(Func,(Func func,A a,B b,C c,D d,E e,F f, G g, H h),(a,b,c,d,e,f,g,h))
   template<class Func,class A,class B,class C,class D,class E,class F, class G>
      Method(Func,(Func func,A a,B b,C c,D d,E e,F f, G g),(a,b,c,d,e,f,g))
   template<class Func,class A,class B,class C,class D,class E,class F>
      Method(Func,(Func func,A a,B b,C c,D d,E e,F f),(a,b,c,d,e,f))
   template<class Func,class A,class B,class C,class D,class E>
      Method(Func,(Func func,A a,B b,C c,D d,E e),(a,b,c,d,e))
   template<class Func,class A,class B,class C,class D>
      Method(Func,(Func func,A a,B b,C c,D d),(a,b,c,d))
   template<class Func,class A,class B,class C>
      Method(Func,(Func func,A a,B b,C c),(a,b,c))
   template<class Func,class A,class B>
      Method(Func,(Func func,A a,B b),(a,b))
   template<class Func,class A>
      Method(Func,(Func func,A a),(a))
   template<class Func>
      Method(Func,(Func func),())
   #undef Method

   /// Journal a function call. Store the function id and all the
   /// function's arguments into the journal. On journal playback the
   /// function is executed with the retrieved arguments. The function
   /// must have been previously declared using the declareFunction()
   /// method.
   #define Method(Arg1,Arg2,Arg3) \
      static void Call Arg1 { \
         if (_Dispatching) \
            return; \
         if (_State == PlayState) \
            return; \
         if (_State == RecordState) { \
            Id id = _getFunctionId((VoidPtr)func,0); \
            AssertFatal(id,"Journal: Function must be be declared before being called"); \
            _start(); \
            IOHelper::writes Arg2; \
            _finish(); \
         } \
         _Dispatching = true; \
         (*func) Arg3; \
         _Dispatching = false; \
         return; \
      }

   template<class Func,class A,class B,class C,class D,class E, class F, class G, class H, class I, class J, class K, class L, class M>
      Method((Func func,A a,B b,C c,D d,E e,F f,G g,H h,I i,J j,K k,L l,M m),(mFile,id,a,b,c,d,e,f,g,h,i,j,k,l,m),(a,b,c,d,e,f,g,h,i,j,k,l,m))
   template<class Func,class A,class B,class C,class D,class E, class F, class G, class H, class I, class J, class K, class L>
      Method((Func func,A a,B b,C c,D d,E e,F f,G g,H h,I i,J j,K k,L l),(mFile,id,a,b,c,d,e,f,g,h,i,j,k,l),(a,b,c,d,e,f,g,h,i,j,k,l))
   template<class Func,class A,class B,class C,class D,class E, class F, class G, class H, class I, class J, class K>
      Method((Func func,A a,B b,C c,D d,E e,F f,G g,H h,I i,J j,K k),(mFile,id,a,b,c,d,e,f,g,h,i,j,k),(a,b,c,d,e,f,g,h,i,j,k))
   template<class Func,class A,class B,class C,class D,class E, class F, class G, class H, class I, class J>
      Method((Func func,A a,B b,C c,D d,E e,F f,G g,H h,I i,J j),(mFile,id,a,b,c,d,e,f,g,h,i,j),(a,b,c,d,e,f,g,h,i,j))
   template<class Func,class A,class B,class C,class D,class E, class F, class G, class H, class I>
      Method((Func func,A a,B b,C c,D d,E e,F f,G g,H h,I i),(mFile,id,a,b,c,d,e,f,g,h,i),(a,b,c,d,e,f,g,h,i))
   template<class Func,class A,class B,class C,class D,class E, class F, class G, class H>
      Method((Func func,A a,B b,C c,D d,E e,F f,G g,H h),(mFile,id,a,b,c,d,e,f,g,h),(a,b,c,d,e,f,g,h))
   template<class Func,class A,class B,class C,class D,class E, class F, class G>
      Method((Func func,A a,B b,C c,D d,E e,F f,G g),(mFile,id,a,b,c,d,e,f,g),(a,b,c,d,e,f,g))
   template<class Func,class A,class B,class C,class D,class E, class F>
      Method((Func func,A a,B b,C c,D d,E e,F f),(mFile,id,a,b,c,d,e,f),(a,b,c,d,e,f))
   template<class Func,class A,class B,class C,class D,class E>
      Method((Func func,A a,B b,C c,D d,E e),(mFile,id,a,b,c,d,e),(a,b,c,d,e))
   template<class Func,class A,class B,class C,class D>
      Method((Func func,A a,B b,C c,D d),(mFile,id,a,b,c,d),(a,b,c,d))
   template<class Func,class A,class B,class C>
      Method((Func func,A a,B b,C c),(mFile,id,a,b,c),(a,b,c))
   template<class Func,class A,class B>
      Method((Func func,A a,B b),(mFile,id,a,b),(a,b))
   template<class Func,class A>
      Method((Func func,A a),(mFile,id,a),(a))
   template<class Func>
      Method((Func func),(mFile,id),())
   #undef Method

   #define Method(Arg1,Arg2,Arg3) \
      static void Call Arg1 { \
         if (_Dispatching) \
            return; \
         if (_State == PlayState) \
            return; \
         if (_State == RecordState) { \
            Id id = _getFunctionId((VoidPtr)obj,(VoidMethod)method); \
            AssertFatal(id != 0,"Journal: Function must be be declared before being called"); \
            _start(); \
            IOHelper::writes Arg2; \
            _finish(); \
         } \
         _Dispatching = true; \
         (obj->*method) Arg3; \
         _Dispatching = false; \
         return; \
      }


   template<class Obj,class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K,class L,class M>
      Method((Obj* obj,void (Obj::*method)(A,B,C,D,E,F,G,H,I,J,K,L,M),A a,B b,C c,D d,E e,F f,G g,H h,I i,J j,K k,L l,M m),(mFile,id,a,b,c,d,e,f,g,h,i,j,k,l,m),(a,b,c,d,e,f,g,h,i,j,k,l,m))
   template<class Obj,class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K,class L>
      Method((Obj* obj,void (Obj::*method)(A,B,C,D,E,F,G,H,I,J,K,L),A a,B b,C c,D d,E e,F f,G g,H h,I i,J j,K k,L l),(mFile,id,a,b,c,d,e,f,g,h,i,j,k,l),(a,b,c,d,e,f,g,h,i,j,k,l))
   template<class Obj,class A,class B,class C,class D,class E,class F,class G,class H,class I,class J,class K>
      Method((Obj* obj,void (Obj::*method)(A,B,C,D,E,F,G,H,I,J,K),A a,B b,C c,D d,E e,F f,G g,H h,I i,J j,K k),(mFile,id,a,b,c,d,e,f,g,h,i,j,k),(a,b,c,d,e,f,g,h,i,j,k))
   template<class Obj,class A,class B,class C,class D,class E,class F,class G,class H,class I,class J>
      Method((Obj* obj,void (Obj::*method)(A,B,C,D,E,F,G,H,I,J),A a,B b,C c,D d,E e,F f,G g,H h,I i,J j),(mFile,id,a,b,c,d,e,f,g,h,i,j),(a,b,c,d,e,f,g,h,i,j))
   template<class Obj,class A,class B,class C,class D,class E,class F,class G,class H,class I>
      Method((Obj* obj,void (Obj::*method)(A,B,C,D,E,F,G,H,I),A a,B b,C c,D d,E e,F f,G g,H h,I i),(mFile,id,a,b,c,d,e,f,g,h,i),(a,b,c,d,e,f,g,h,i))
   template<class Obj,class A,class B,class C,class D,class E,class F,class G,class H>
      Method((Obj* obj,void (Obj::*method)(A,B,C,D,E,F,G,H),A a,B b,C c,D d,E e,F f,G g,H h),(mFile,id,a,b,c,d,e,f,g,h),(a,b,c,d,e,f,g,h))
   template<class Obj,class A,class B,class C,class D,class E,class F,class G>
      Method((Obj* obj,void (Obj::*method)(A,B,C,D,E,F,G),A a,B b,C c,D d,E e,F f,G g),(mFile,id,a,b,c,d,e,f,g),(a,b,c,d,e,f,g))
   template<class Obj,class A,class B,class C,class D,class E,class F>
      Method((Obj* obj,void (Obj::*method)(A,B,C,D,E,F),A a,B b,C c,D d,E e,F f),(mFile,id,a,b,c,d,e,f),(a,b,c,d,e,f))
   template<class Obj,class A,class B,class C,class D,class E>
      Method((Obj* obj,void (Obj::*method)(A,B,C,D,E),A a,B b,C c,D d,E e),(mFile,id,a,b,c,d,e),(a,b,c,d,e))
   template<class Obj,class A,class B,class C,class D>
      Method((Obj* obj,void (Obj::*method)(A,B,C,D),A a,B b,C c,D d),(mFile,id,a,b,c,d),(a,b,c,d))
   template<class Obj,class A,class B,class C>
      Method((Obj* obj,void (Obj::*method)(A,B,C),A a,B b,C c),(mFile,id,a,b,c),(a,b,c))
   template<class Obj,class A,class B>
      Method((Obj* obj,void (Obj::*method)(A,B),A a,B b),(mFile,id,a,b),(a,b))
   template<class Obj,class A>
      Method((Obj* obj,void (Obj::*method)(A),A a),(mFile,id,a),(a))
   template<class Obj>
      Method((Obj* obj,void (Obj::*method)()),(mFile,id),())

   #undef Method

   /// Write data into the journal. Non-deterministic data can be stored
   /// into the journal for reading during playback. The function
   /// returns true if the journal is record mode.
   #define Method(Arg1,Arg2) \
      static inline bool Writes Arg1 { \
         if (_State == RecordState) { \
            _start(); IOHelper::writes Arg2; _finish(); \
            return true; \
         } \
         return false; \
      }

   template<class A,class B,class C,class D,class E, class F, class G, class H,class I,class J,class K,class L,class M>
      Method((A& a,B& b,C& c,D& d,E& e, F& f, G& g, H& h, I& i,J& j,K& k,L& l,M& m),(mFile,a,b,c,d,e,f,g,h,i,j,k,l,m));
   template<class A,class B,class C,class D,class E, class F, class G, class H,class I,class J,class K,class L>
      Method((A& a,B& b,C& c,D& d,E& e, F& f, G& g, H& h, I& i,J& j,K& k,L& l),(mFile,a,b,c,d,e,f,g,h,i,j,k,l));
   template<class A,class B,class C,class D,class E, class F, class G, class H,class I,class J,class K>
      Method((A& a,B& b,C& c,D& d,E& e, F& f, G& g, H& h, I& i,J& j,K& k),(mFile,a,b,c,d,e,f,g,h,i,j,k));
   template<class A,class B,class C,class D,class E, class F, class G, class H,class I,class J>
      Method((A& a,B& b,C& c,D& d,E& e, F& f, G& g, H& h, I& i,J& j),(mFile,a,b,c,d,e,f,g,h,i,j));
   template<class A,class B,class C,class D,class E, class F, class G, class H,class I>
      Method((A& a,B& b,C& c,D& d,E& e, F& f, G& g, H& h, I& i),(mFile,a,b,c,d,e,f,g,h,i));
   template<class A,class B,class C,class D,class E, class F, class G, class H>
      Method((A& a,B& b,C& c,D& d,E& e, F& f, G& g, H& h),(mFile,a,b,c,d,e,f,g,h));
   template<class A,class B,class C,class D,class E, class F, class G>
      Method((A& a,B& b,C& c,D& d,E& e, F& f, G& g),(mFile,a,b,c,d,e,f,g));
   template<class A,class B,class C,class D,class E, class F>
      Method((A& a,B& b,C& c,D& d,E& e, F& f),(mFile,a,b,c,d,e,f));
   template<class A,class B,class C,class D,class E>
      Method((A& a,B& b,C& c,D& d,E& e),(mFile,a,b,c,d,e));
   template<class A,class B,class C,class D>
      Method((A& a,B& b,C& c,D& d),(mFile,a,b,c,d));
   template<class A,class B,class C>
      Method((A& a,B& b,C& c),(mFile,a,b,c));
   template<class A,class B>
      Method((A& a,B& b),(mFile,a,b));
   template<class A>
      Method((A& a),(mFile,a));
   #undef Method

   /// Read data from the journal. Read non-deterministic data stored
   /// during the recording phase. The function returns true if the
   /// journal is play mode.
   #define Method(Arg1,Arg2) \
      static inline bool Reads Arg1 { \
         if (_State == PlayState) { \
            _start(); IOHelper::reads Arg2; _finish(); \
         return true; \
         } \
         return false; \
      }

   template<class A,class B,class C,class D,class E, class F, class G, class H, class I, class J, class K,class L,class M>
      Method((A& a,B& b,C& c,D& d,E& e,F& f,G& g, H& h, I& i, J& j, K& k, L& l, M& m),(mFile,a,b,c,d,e,f,g,h,i,j,k,l,m));
   template<class A,class B,class C,class D,class E, class F, class G, class H, class I, class J, class K,class L>
      Method((A& a,B& b,C& c,D& d,E& e,F& f,G& g, H& h, I& i, J& j, K& k, L& l),(mFile,a,b,c,d,e,f,g,h,i,j,k,l));
   template<class A,class B,class C,class D,class E, class F, class G, class H, class I, class J, class K>
      Method((A& a,B& b,C& c,D& d,E& e,F& f,G& g, H& h, I& i, J& j, K& k),(mFile,a,b,c,d,e,f,g,h,i,j,k));
   template<class A,class B,class C,class D,class E, class F, class G, class H, class I, class J>
      Method((A& a,B& b,C& c,D& d,E& e,F& f,G& g, H& h, I& i, J& j),(mFile,a,b,c,d,e,f,g,h,i,j));
   template<class A,class B,class C,class D,class E, class F, class G, class H, class I>
      Method((A& a,B& b,C& c,D& d,E& e,F& f,G& g, H& h, I& i),(mFile,a,b,c,d,e,f,g,h,i));
   template<class A,class B,class C,class D,class E, class F, class G, class H>
      Method((A& a,B& b,C& c,D& d,E& e,F& f,G& g, H& h),(mFile,a,b,c,d,e,f,g,h));
   template<class A,class B,class C,class D,class E, class F, class G>
      Method((A& a,B& b,C& c,D& d,E& e,F& f,G& g),(mFile,a,b,c,d,e,f,g));
   template<class A,class B,class C,class D,class E, class F>
      Method((A& a,B& b,C& c,D& d,E& e,F& f),(mFile,a,b,c,d,e,f));
   template<class A,class B,class C,class D,class E>
      Method((A& a,B& b,C& c,D& d,E& e),(mFile,a,b,c,d,e));
   template<class A,class B,class C,class D>
      Method((A& a,B& b,C& c,D& d),(mFile,a,b,c,d));
   template<class A,class B,class C>
      Method((A& a,B& b,C& c),(mFile,a,b,c));
   template<class A,class B>
      Method((A& a,B& b),(mFile,a,b));
   template<class A>
      Method((A& a),(mFile,a));

   #undef Method
};

#endif
