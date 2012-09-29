#ifndef BOOST_SMART_PTR_WEAK_PTR_HPP_INCLUDED
#define BOOST_SMART_PTR_WEAK_PTR_HPP_INCLUDED

//
//  weak_ptr.hpp
//
//  Copyright (c) 2001, 2002, 2003 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/weak_ptr.htm for documentation.
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
	   }

	public:	
		
		WeakRefPtr<AnyStrongRefBase> w_ref;

		WeakRefBase::WeakReference* get_ref() const { if(w_ref.getPointer()) return w_ref.getPointer()->getWeakReference(); else return NULL;}
		T* get_real_ptr() const { if(w_ref.getPointer()) return (T*)w_ref.getPointer()->_px; return NULL;}

		~AnyWeakRefPtr()
		{
			//set( NULL ); //TODO check
		}

		AnyWeakRefPtr() : w_ref(NULL) {}			

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

		/*template<typename Y>
		AnyWeakRefPtr<T>& operator=( const AnyStrongRefPtr<Y>& p )
		{
			set( p.ref );
			return *this;
		}*/

		template<typename Y>
		AnyWeakRefPtr<T>& operator=( const AnyWeakRefPtr<Y>& p )
		{
			set( p.w_ref );
			return *this;
		}

		template<typename Y>
		bool operator<( const AnyWeakRefPtr<Y>& p ) const
		{
			return get_ref() < p.get_ref();
		}

		long use_count() const
		{
			if( w_ref.isNull() )
				return 0;

			return w_ref.getPointer()->getRefCount();
		}

	};


template<class T> class weak_ptr
{
private:
    
    typedef weak_ptr<T> this_type;

public:

    typedef T element_type;
	typedef weak_ptr<T> type;	

    weak_ptr(): pn()
    {
    }

	template<class Y>
    weak_ptr( weak_ptr<Y> const & r ) : pn( r.lock().pn ) // never throws
    {		
    }

    template<class Y>
    weak_ptr( weak_ptr<Y> && r ) : pn(  r.lock().pn  ) // never throws
    {
        r.px = NULL;
    }

    // for better efficiency in the T == Y case
    weak_ptr( weak_ptr && r ): pn( r.pn ) // never throws
    {
        r.px = NULL;
    }

    // for better efficiency in the T == Y case
    weak_ptr & operator=( weak_ptr && r ) // never throws
    {
        this_type( static_cast< weak_ptr && >( r ) ).swap( *this );
        return *this;
    }

	template< typename Y >
	weak_ptr( Torque::tSharedPtr<Y> const & r ) :  pn( r.pn ) // never throws
    {
    }

#if !defined(BOOST_MSVC) || (BOOST_MSVC >= 1300)

    template<class Y>
    weak_ptr & operator=(weak_ptr<Y> const & r) // never throws
    {      
        pn = r.lock().pn;
        return *this;
    }

    template<class Y>
    weak_ptr & operator=(shared_ptr<Y> const & r) // never throws
    {        
        pn = r.pn.mObject;
        return *this;
    }

#endif

    typename shared_ptr<T>::type lock() const // never throws
    {
		if( !pn.get_ref() )
			return shared_ptr<T>::type();
		
        return tSharedPtr<T>( AnyStrongRefPtr<T>(pn) );
    }

    long use_count() const // never throws
    {		
        return pn.use_count();
    }

    bool expired() const // never throws
    {
		return pn.w_ref.isNull();
    }

    bool _empty() const // extension, not in std::weak_ptr
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

    template<class Y> bool owner_before( weak_ptr<Y> const & rhs ) const
    {
        return pn < rhs.pn;
    }

    template<class Y> bool owner_before( shared_ptr<Y> const & rhs ) const
    {
        return pn < rhs.pn;
    }
    
    AnyWeakRefPtr<T> pn; // reference counter

};  // weak_ptr

template<class T, class U> inline bool operator<(weak_ptr<T> const & a, weak_ptr<U> const & b)
{
    return a.owner_before( b );
}

template<class T> void swap(weak_ptr<T> & a, weak_ptr<T> & b)
{
    a.swap(b);
}

}; // namespace boost




#endif  // #ifndef BOOST_SMART_PTR_WEAK_PTR_HPP_INCLUDED


