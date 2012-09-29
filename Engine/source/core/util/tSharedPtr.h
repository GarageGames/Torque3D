#ifndef _T3D_SHARED_PTR_H_
#define _T3D_SHARED_PTR_H_

//
//  Copyright (c) 2001, 2002, 2003 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/WeakPtr.htm for documentation.
//

#include "core/util/refBase.h"
#include "platform\platformAssert.h"
#include "core\util\autoPtr.h"

namespace Torque
{
	template<class T> class WeakPtr;
	template<typename T> class AnyWeakRefPtr;
	
	namespace detail {
		struct dynamic_cast_tag {};
		struct static_cast_tag {};
	};	

	class AnyStrongRefBase : public StrongRefBase
	{		
	protected:
		AnyStrongRefBase() : _px(NULL) {}
	public:
		void* _px;
	};

	template<typename Y>
	class AnyStrongRef : public AnyStrongRefBase
	{	
	public:
		AnyStrongRef() : AnyStrongRefBase(), _destructor(NULL) {}
		virtual void destroySelf() 
		{ 
			if(_px && _destructor)
				_destructor(( Y*)_px );
			else delete (Y*)(_px); 

			delete this; 
		}
		Y* get_ptr() const { return (Y*)_px; }		

		void (*_destructor)(Y*);
	};

	template<typename T>
	class AnyStrongRefPtr
	{
		void set(StrongRefPtr< AnyStrongRefBase > const & n_ref)
		{
			ref = n_ref;		
		}		

	public:	

		typedef WeakRefPtr<AnyStrongRefBase> weak_ptr_internal_ptr_type;
		StrongRefPtr< AnyStrongRefBase > ref;	

		~AnyStrongRefPtr()
		{
			set( StrongRefPtr< AnyStrongRefBase >() );
		}

		AnyStrongRefPtr() :  ref(nullptr) {}

		template<typename Y>
		AnyStrongRefPtr(Y* p)
		{	
			ref = new AnyStrongRef<Y>();
			ref->_px = p;	
		}	

		template<typename Y, typename D>
		AnyStrongRefPtr(Y* p, D* d)
		{	
			auto new_ref = new AnyStrongRef<Y>();
			new_ref->_px = p;	
			new_ref->_destructor = d;
			ref = new_ref;
		}	

		template<typename Y>
		AnyStrongRefPtr( AnyStrongRefPtr<Y> const & p) 
		{			
			ref = p.ref;
		}

	
		AnyStrongRefPtr( AnyStrongRefPtr const & p) 
		{		
			set( p.ref );
		}

		template<typename Y>
		AnyStrongRefPtr( AnyWeakRefPtr<Y> const &p) 
		{		
			if( !p.use_count() )
				throw( bad_weak_ptr() );

			set( p.w_ref.getPointer() );
		}

		T* get_real_ptr() const 
		{
			if( ref.getPointer() )
				return (T*)ref.getPointer()->_px;
			else
				return NULL;
		}

		void swap( AnyStrongRefPtr<T>& p)
		{
			AnyStrongRefPtr<T> temp = *this;
			*this = p;
			p = temp;
		}
	
		AnyStrongRefPtr& operator=( const AnyStrongRefPtr& p )
		{
			set( p.ref.getPointer() );

			return *this;
		}

		template<typename Y>
		AnyStrongRefPtr<T>& operator=( const AnyStrongRefPtr<Y>& p )
		{
			set( p.ref.getPointer() );

			return *this;
		}

		template<typename Y>
		bool operator<( const AnyStrongRefPtr<Y>& p ) const
		{			
			return ref.getPointer()->getWeakReference() < p.ref.getPointer()->getWeakReference();
		}

		long use_count() const
		{
			return ref->getRefCount();
		}

	};


	template<class T> class SharedPtr
	{
	private:
   
		typedef SharedPtr<T> this_type;

		template<typename Y>
		struct return_reference
		{
			typedef Y & reference;
		};

		template<>
		struct return_reference<void>
		{
			typedef void reference;
		};

		typedef typename return_reference<T>::reference reference;
		typedef T * this_type::*unspecified_bool_type;
	

		template<class Y> friend class SharedPtr;
		template<class Y> friend class WeakPtr;

	public:

		typedef T element_type;
		typedef T value_type;
		typedef T * pointer;
    

		SharedPtr():  pn() // never throws in 1.30+
		{
		}

		SharedPtr( const AnyStrongRefPtr<T> &r ): pn( r ) // never throws
		{
		} 

		template<class Y>
		explicit SharedPtr( Y * p ): pn( p ) // Y must be complete
		{       
		}   

		template<class Y, class D>
		SharedPtr( Y * p, D* d ): pn( p, d ) // Y must be complete
		{       
		} 

		SharedPtr( SharedPtr const & r ): pn( r.pn ) // never throws
		{
		}  

		template<typename Y>
		SharedPtr( SharedPtr<Y> const & r ): pn( r.pn ) // never throws
		{
		}  

		template<class Y>
		explicit SharedPtr(WeakPtr<Y> const & r): pn(r.pn) // may throw
		{
			
		}

		template<class Y>
		explicit SharedPtr( AutoPtr<Y> & r ) : pn( SharedPtr<Y>( r.release() ).pn )
		{

		}

		//dynamic_cast_tag
		template<class Y>
		SharedPtr(SharedPtr<Y> const & r, detail::dynamic_cast_tag): pn(r.pn)
		{		
			//if( !dynamic_cast<element_type*>( r.pn.ref.getPointer() ) ) // need to allocate new counter -- the cast failed
			if( !dynamic_cast<T*>( r.pn.get_real_ptr() ) )
			{
				pn = AnyStrongRefPtr<T>();
			}
		
		}

		template<class Y>
		SharedPtr(SharedPtr<Y> const & r, detail::static_cast_tag) 
		{
			pn = static_cast< AnyStrongRefPtr<Y> >(r.pn);		
		}
	 

		// assignment

		SharedPtr & operator=( SharedPtr const & r ) // never throws
		{
			this_type(r).swap(*this);
			return *this;
		}	

		template<class Y>
		SharedPtr & operator=( AutoPtr<Y> & r )
		{
			this_type(r).swap(*this);
			return *this;
		}

		template<class Y>
		SharedPtr & operator=( Torque::WeakPtr<Y> & r )
		{
			this_type(r).swap(*this);
			return *this;
		}

		void reset() // never throws in 1.30+
		{
			this_type().swap(*this);
		}

		template<class Y> void reset(Y * p) // Y must be complete
		{
			AssertFatal(p == 0 || p != px, ""); // catch self-reset errors
			this_type(p).swap(*this);
		}

		template<class Y, class D> void reset( Y * p, D d )
		{
			this_type( p, d ).swap( *this );
		}

		template<class Y, class D, class A> void reset( Y * p, D d, A a )
		{
			this_type( p, d, a ).swap( *this );
		}

		template<class Y> void reset( SharedPtr<Y> const & r, T * p )
		{
			this_type( r, p ).swap( *this );
		}

		reference operator* () const // never throws
		{
			AssertFatal(pn.get_real_ptr() != 0, "");
			return *(pn.get_real_ptr());
		}

		T * operator-> () const // never throws
		{
			AssertFatal(pn.get_real_ptr() != 0,"");
			return pn.get_real_ptr();
		}

		T * get() const // never throws
		{
			return pn.get_real_ptr();
		}
		
		operator unspecified_bool_type() const // never throws
		{		
			return pn.get_real_ptr() == 0? 0: &this_type::__px;
		}

		bool unique() const // never throws
		{
			return pn.unique();
		}

		long use_count() const // never throws
		{
			return pn.use_count();
		}

		void swap(SharedPtr<T> & other) // never throws
		{  
			pn.swap(other.pn);
		}

		void swap(Torque::WeakPtr<T> & other) // never throws
		{    
			pn.swap(other.pn);
		}

		template<class Y> bool owner_before( SharedPtr<Y> const & rhs ) const
		{
			return pn < rhs.pn;
		}

		bool _internal_equiv( SharedPtr const & r ) const
		{
		
			return px == r.px && pn == r.pn;
		}

	private:

		T * __px;                     // contained pointer
		AnyStrongRefPtr<T> pn;		  // reference counter

	};  // SharedPtr


	template<class T, class U> inline bool operator==(SharedPtr<T> const & a, SharedPtr<U> const & b)
	{
		return a.get() == b.get();
	}

	template<class T, class U> inline bool operator!=(SharedPtr<T> const & a, SharedPtr<U> const & b)
	{
		return a.get() != b.get();
	}

	template<class T, class U> SharedPtr<T> dynamic_pointer_cast(SharedPtr<U> const & r)
	{
		return SharedPtr<T>(r, Torque::detail::dynamic_cast_tag());
	}

	template<class T, class U> SharedPtr<T> static_pointer_cast(SharedPtr<U> const & r)
	{
		return SharedPtr<T>(r, Torque::detail::static_cast_tag());
	}

	template<class T, class U> inline bool operator<(SharedPtr<T> const & a, SharedPtr<U> const & b)
	{
		return a.owner_before( b );
	}

};

#endif 