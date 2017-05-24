#ifndef _TORQUE_WEAK_PTR_H_
#define _TORQUE_WEAK_PTR_H_

//
//  Copyright (c) 2001, 2002, 2003 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/WeakPtr.htm for documentation.
//

#include "core\util\refBase.h"
#include "core\util\tSharedPtr.h"

namespace Torque
{
	template<typename T>
	class AnyWeakRefPtr
	{
		void set( WeakRefPtr<AnyStrongRefBase> const &_ref)
	   {	
		  w_ref = _ref;
		  if( _ref.isValid() )
			_last_ref = _ref->getWeakReference();
		  else
			  _last_ref = NULL;
	   }
		
	public:	

		void* _last_ref; //TODO to private
		
		WeakRefPtr<AnyStrongRefBase> w_ref;

		WeakRefBase::WeakReference* get_ref() const { if(w_ref.getPointer()) return w_ref.getPointer()->getWeakReference(); else return NULL;}
		T* get_real_ptr() const { if(w_ref.getPointer()) return (T*)w_ref.getPointer()->_px; return NULL;}

		~AnyWeakRefPtr()
		{
			
		}

		AnyWeakRefPtr() : w_ref(NULL), _last_ref(NULL) {}			

		template<typename Y>
		AnyWeakRefPtr(AnyWeakRefPtr<Y> const &p)
		{		
			set( p.w_ref );
		}

		template< typename Y >
		AnyWeakRefPtr(AnyStrongRefPtr<Y> const &p) 
		{		
			(T*)p.get_real_ptr();			
			set( p.ref.getPointer() );
		}

		void swap( AnyWeakRefPtr<T>& p)
		{
			AnyWeakRefPtr<T> temp = *this;			
			*this = p;
			p = temp;
		}

		template<typename Y>
		AnyWeakRefPtr<T>& operator=( const AnyStrongRefPtr<Y>& p )
		{
			WeakRefPtr<AnyStrongRefBase> temp_wref = p.ref.getPointer();
			set( temp_wref );
			return *this;
		}

		template<typename Y>
		AnyWeakRefPtr<T>& operator=( const AnyWeakRefPtr<Y>& p )
		{
			set( p.w_ref );
			return *this;
		}

		template<typename Y>
		bool operator<( const AnyWeakRefPtr<Y>& p ) const
		{
			return _last_ref < p._last_ref;
		}

		long use_count() const
		{
			if( w_ref.isNull() )
				return 0;

			return w_ref.getPointer()->getRefCount();
		}

	};

	struct bad_weak_ptr {};


	template<class T> 
	class WeakPtr
	{
	private:
    
		typedef WeakPtr<T> this_type;

		template<class Y> friend class SharedPtr;
		template<class Y> friend class WeakPtr;

	public:

		typedef T element_type;
		typedef WeakPtr<T> type;	

		WeakPtr(): pn()
		{
		}

		template<class Y>
		WeakPtr( WeakPtr<Y> const & r ) : pn( r.lock().pn ) // never throws
		{		
		}

		template<class Y>
		WeakPtr( WeakPtr<Y> && r ) : pn(  r.lock().pn  ) // never throws
		{
			r.px = NULL;
		}

		// for better efficiency in the T == Y case
		WeakPtr( WeakPtr && r ): pn( r.pn ) // never throws
		{
			r.px = NULL;
		}

		template< typename Y >
		WeakPtr( Torque::SharedPtr<Y> const & r ) :  pn( r.pn ) // never throws
		{
		}

		// for better efficiency in the T == Y case
		WeakPtr & operator=( WeakPtr && r ) // never throws
		{
			this_type( static_cast< WeakPtr && >( r ) ).swap( *this );
			return *this;
		}		

		template<class Y>
		WeakPtr & operator=(WeakPtr<Y> const & r) // never throws
		{      
			pn = r.lock().pn;
			return *this;
		}

		template<class Y>
		WeakPtr & operator=(SharedPtr<Y> const & r) // never throws
		{        
			pn = r.pn;
			return *this;
		}

		SharedPtr<T> lock() const // never throws
		{
			if( !pn.get_ref() )
				return SharedPtr<T>();
		
			return SharedPtr<T>( AnyStrongRefPtr<T>(pn) );
		}

		long use_count() const // never throws
		{		
			return pn.use_count();
		}

		bool expired() const // never throws
		{
			return pn.w_ref.isNull();
		}

		bool _empty() const // extension, not in std::WeakPtr
		{
			return pn.empty();
		}

		void reset() // never throws in 1.30+
		{
			this_type().swap(*this);
		}

		void swap(this_type & other) // never throws
		{      
			pn.swap(other.pn);
		}

		void _internal_assign(T * px2, AnyWeakRefPtr<T> const & pn2)
		{       
			pn = pn2;
		}

		template<class Y> bool owner_before( WeakPtr<Y> const & rhs ) const
		{
			return pn < rhs.pn;
		}

		template<class Y> bool owner_before( SharedPtr<Y> const & rhs ) const
		{
			return pn < rhs.pn;
		}

	private:
    
		AnyWeakRefPtr<T> pn; // reference counter

	};  // WeakPtr

	template<class T, class U> inline bool operator<(WeakPtr<T> const & a, WeakPtr<U> const & b)
	{
		return a.owner_before( b );
	}

	template<class T> void swap(WeakPtr<T> & a, WeakPtr<T> & b)
	{
		a.swap(b);
	}

}; // namespace Torque




#endif  // #ifndef _TORQUE_WEAK_PTR_H_


