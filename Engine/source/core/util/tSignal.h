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

#ifndef _TSIGNAL_H_
#define _TSIGNAL_H_

#ifndef _UTIL_DELEGATE_H_
#include "core/util/delegate.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

/// Signals (Multi-cast Delegates)
/// 
/// Signals are used throughout this engine to allow subscribers to listen
/// for generated events for various things.  
/// 
/// Signals are called according to their order parameter (lower
/// numbers first).
///
/// Signal functions can return bool or void.  If bool then returning false
/// from a signal function will cause entries in the ordered signal list after 
/// that one to not be called.
///
/// This allows handlers of a given event to say to the signal generator, "I handled this message, and 
/// it is no longer appropriate for other listeners to handle it"

class SignalBase
{
public:

   SignalBase()
   {
      mList.next = mList.prev = &mList;
      mList.order = 0.5f;
      mTriggerNext = NULL;
   }

   ~SignalBase();

   /// Removes all the delegates from the signal.
   void removeAll();

   /// Returns true if the delegate list is empty.
   bool isEmpty() const
   {
      return mList.next == &mList;
   }

protected:

   struct DelegateLink
   {
      DelegateLink *next,*prev;
      F32 order;

      void insert(DelegateLink* node, F32 order);
      void unlink();
   };

   DelegateLink mList;

   /// We need to protect the delegate list against removal of the currently
   /// triggering node as well removal of the next node in the list.  When not
   /// handling these two cases correctly, removing delegates from a signal
   /// while it is triggering will lead to crashes.
   ///
   /// So this field stores the next node of each active traversal so that when
   /// we unlink a node, we can check it against this field and move the traversal
   /// along if needed.
   Vector<DelegateLink*> mTriggerNext;
};

template<typename Signature> class SignalBaseT : public SignalBase
{
public:

   /// The delegate signature for this signal.
   typedef Delegate<Signature> DelegateSig;

   SignalBaseT() {}

   SignalBaseT( const SignalBaseT &base )
   {
      mList.next = mList.prev = &mList;
      merge( base );
   }

   void operator =( const SignalBaseT &base )
   {
      removeAll();
      merge( base );
   }

   void merge( const SignalBaseT &base )
   {
      for ( DelegateLink *ptr = base.mList.next; ptr != &base.mList; ptr = ptr->next )
      {
         DelegateLinkImpl *del = static_cast<DelegateLinkImpl*>( ptr );
         notify( del->mDelegate, del->order );
      }
   }

   void notify( const DelegateSig &dlg, F32 order = 0.5f)
   {
      mList.insert(new DelegateLinkImpl(dlg), order);
   }

   void remove( DelegateSig dlg )
   {
      for( DelegateLink* ptr = mList.next;ptr != &mList; ptr = ptr->next )
      {
         if( DelegateLinkImpl* del = static_cast< DelegateLinkImpl* >( ptr ) )
         {
            if( del->mDelegate == dlg )
            {
               for ( int i = 0; i < mTriggerNext.size(); i++ )
               {
                  if( mTriggerNext[i] == ptr )
                     mTriggerNext[i] = ptr->next;
               }

               del->unlink();
               delete del;
               return;
            }
         }
      }
   } 

   template <class T,class U>
   void notify(T obj,U func, F32 order = 0.5f)
   {
      DelegateSig dlg(obj, func);
      notify(dlg, order);
   }

   template <class T>
   void notify(T func, F32 order = 0.5f)
   {
      DelegateSig dlg(func);
      notify(dlg, order);
   }

   template <class T,class U>
   void remove(T obj,U func)
   {
      DelegateSig compDelegate(obj, func);
      remove(compDelegate);
   }

   template <class T>
   void remove(T func)
   {
      DelegateSig compDelegate(func);
      remove(compDelegate);
   } 

   /// Returns true if the signal already contains this delegate.
   bool contains( const DelegateSig &dlg ) const
   {
      for ( DelegateLink *ptr = mList.next; ptr != &mList; ptr = ptr->next )
      {
         DelegateLinkImpl *del = static_cast<DelegateLinkImpl*>( ptr );
         if ( del->mDelegate == dlg )
            return true;
      }

      return false;
   } 

protected:

   struct DelegateLinkImpl : public SignalBase::DelegateLink
   {
      DelegateSig mDelegate;
      DelegateLinkImpl(DelegateSig dlg) : mDelegate(dlg) {}
   };

   DelegateSig & getDelegate(SignalBase::DelegateLink * link)
   {
      return ((DelegateLinkImpl*)link)->mDelegate;
   }
};

//-----------------------------------------------------------------------------

template<typename Signature> class Signal;

// Short-circuit signal implementations

template<> 
class Signal<bool()> : public SignalBaseT<bool()>
{
   public:

      bool trigger()
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )() )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A> 
class Signal<bool(A)> : public SignalBaseT<bool(A)>
{   
   public:

      bool trigger( A a )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B>
class Signal<bool(A,B)> : public SignalBaseT<bool(A,B)>
{
   public:

      bool trigger( A a, B b )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C> 
class Signal<bool(A,B,C)> : public SignalBaseT<bool(A,B,C)>
{
   public:

      bool trigger( A a, B b, C c )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C, class D> 
class Signal<bool(A,B,C,D)> : public SignalBaseT<bool(A,B,C,D)>
{
   public:

      bool trigger( A a, B b, C c, D d )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c, d ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C, class D, class E> 
class Signal<bool(A,B,C,D,E)> : public SignalBaseT<bool(A,B,C,D,E)>
{
   public:

      bool trigger( A a, B b, C c, D d, E e )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c, d, e ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C, class D, class E, class F> 
class Signal<bool(A,B,C,D,E,F)> : public SignalBaseT<bool(A,B,C,D,E,F)>
{
   public:

      bool trigger( A a, B b, C c, D d, E e, F f )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c, d, e, f ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C, class D, class E, class F, class G> 
class Signal<bool(A,B,C,D,E,F,G)> : public SignalBaseT<bool(A,B,C,D,E,F,G)>
{
   public:

      bool trigger( A a, B b, C c, D d, E e, F f, G g )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c, d, e, f, g ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H> 
class Signal<bool(A,B,C,D,E,F,G,H)> : public SignalBaseT<bool(A,B,C,D,E,F,G,H)>
{
   public:

      bool trigger( A a, B b, C c, D d, E e, F f, G g, H h )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c, d, e, f, g, h ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I> 
class Signal<bool(A,B,C,D,E,F,G,H,I)> : public SignalBaseT<bool(A,B,C,D,E,F,G,H,I)>
{
   public:

      bool trigger( A a, B b, C c, D d, E e, F f, G g, H h, I i )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c, d, e, f, g, h, i ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J> 
class Signal<bool(A,B,C,D,E,F,G,H,I,J)> : public SignalBaseT<bool(A,B,C,D,E,F,G,H,I,J)>
{
   public:

      bool trigger( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c, d, e, f, g, h, i, j ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K> 
class Signal<bool(A,B,C,D,E,F,G,H,I,J,K)> : public SignalBaseT<bool(A,B,C,D,E,F,G,H,I,J,K)>
{
   public:

      bool trigger( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c, d, e, f, g, h, i, j, k ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L> 
class Signal<bool(A,B,C,D,E,F,G,H,I,J,K,L)> : public SignalBaseT<bool(A,B,C,D,E,F,G,H,I,J,K,L)>
{
   public:

      bool trigger( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c, d, e, f, g, h, i, j, k, l ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M> 
class Signal<bool(A,B,C,D,E,F,G,H,I,J,K,L,M)> : public SignalBaseT<bool(A,B,C,D,E,F,G,H,I,J,K,L,M)>
{
   public:

      bool trigger( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            if( !this->getDelegate( ptr )( a, b, c, d, e, f, g, h, i, j, k, l, m ) )
            {
               this->mTriggerNext.pop_back();
               return false;
            }
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
         return true;
      }
};

// Non short-circuit signal implementations

template<> 
class Signal<void()> : public SignalBaseT<void()>
{
   public:

      void trigger()
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )();
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A>
class Signal<void(A)> : public SignalBaseT<void(A)>
{
   public:

      void trigger( A a )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B> 
class Signal<void(A,B)> : public SignalBaseT<void(A,B)>
{
   public:

     void trigger( A a, B b )
     {
        this->mTriggerNext.push_back(NULL);
        for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
        {
           this->mTriggerNext.last() = ptr->next;
           this->getDelegate( ptr )( a, b );
           ptr = this->mTriggerNext.last();
        }
        this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C> 
class Signal<void(A,B,C)> : public SignalBaseT<void(A,B,C)>
{
   public:

      void trigger( A a, B b, C c )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C, class D>
class Signal<void(A,B,C,D)> : public SignalBaseT<void(A,B,C,D)>
{
   public:

      void trigger( A a, B b, C c, D d )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c, d );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C, class D, class E> 
class Signal<void(A,B,C,D,E)> : public SignalBaseT<void(A,B,C,D,E)>
{
   public:

      void trigger( A a, B b, C c, D d, E e )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c, d, e );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C, class D, class E, class F> 
class Signal<void(A,B,C,D,E,F)> : public SignalBaseT<void(A,B,C,D,E,F)>
{
   public:

      void trigger( A a, B b, C c, D d, E e, F f )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c, d, e, f );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C, class D, class E, class F, class G> 
class Signal<void(A,B,C,D,E,F,G)> : public SignalBaseT<void(A,B,C,D,E,F,G)>
{
   public:

      void trigger( A a, B b, C c, D d, E e, F f, G g )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c, d, e, f, g );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H> 
class Signal<void(A,B,C,D,E,F,G,H)> : public SignalBaseT<void(A,B,C,D,E,F,G,H)>
{
   public:

      void trigger( A a, B b, C c, D d, E e, F f, G g, H h )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c, d, e, f, g, h );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I> 
class Signal<void(A,B,C,D,E,F,G,H,I)> : public SignalBaseT<void(A,B,C,D,E,F,G,H,I)>
{
   public:

      void trigger( A a, B b, C c, D d, E e, F f, G g, H h, I i )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c, d, e, f, g, h, i );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J> 
class Signal<void(A,B,C,D,E,F,G,H,I,J)> : public SignalBaseT<void(A,B,C,D,E,F,G,H,I,J)>
{
   public:

      void trigger( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c, d, e, f, g, h, i, j );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K> 
class Signal<void(A,B,C,D,E,F,G,H,I,J,K)> : public SignalBaseT<void(A,B,C,D,E,F,G,H,I,J,K)>
{
   public:

      void trigger( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c, d, e, f, g, h, i, j, k );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L> 
class Signal<void(A,B,C,D,E,F,G,H,I,J,K,L)> : public SignalBaseT<void(A,B,C,D,E,F,G,H,I,J,K,L)>
{
   public:

      void trigger( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c, d, e, f, g, h, i, j, k, l );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M> 
class Signal<void(A,B,C,D,E,F,G,H,I,J,K,L,M)> : public SignalBaseT<void(A,B,C,D,E,F,G,H,I,J,K,L,M)>
{
   public:

      void trigger( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m )
      {
         this->mTriggerNext.push_back(NULL);
         for( SignalBase::DelegateLink* ptr = this->mList.next; ptr != &this->mList; )
         {
            this->mTriggerNext.last() = ptr->next;
            this->getDelegate( ptr )( a, b, c, d, e, f, g, h, i, j, k, l, m );
            ptr = this->mTriggerNext.last();
         }
         this->mTriggerNext.pop_back();
      }
};

#endif // _TSIGNAL_H_
