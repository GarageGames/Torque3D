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
		void set(T* _px, WeakRefBase * _ref)
	   {
		  if (ref)
			 ref->getWeakReference()->decRefCount();
		  ref = NULL;
		  px = NULL;
		  if (_ref)
		  {
			 ref = _ref;
			 ref->getWeakReference()->incRefCount();
			 px = _px;
		  }
	   }

	public:	

		T* px;
		WeakRefBase *ref;

		WeakRefBase* get_ref() const { return ref; }
		~AnyWeakRefPtr()
		{
			set( NULL, NULL );
		}

		AnyWeakRefPtr() : px(NULL), ref(NULL) {}			

		template<typename Y>
		AnyWeakRefPtr(const AnyWeakRefPtr<Y>& p)
		{		
			set( p.px, p.ref );
		}

		template< typename Y >
		AnyWeakRefPtr(const AnyStrongRefPtr<Y>& p)
		{		
			set( p.px, p.ref );
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
			set( p.px, p.ref );
			return *this;
		}

		template<typename Y>
		AnyWeakRefPtr<T>& operator=( const AnyWeakRefPtr<Y>& p )
		{
			set( p.px, p.ref );
			return *this;
		}

	};


template<class T> class weak_ptr
{
private:
    
    typedef weak_ptr<T> this_type;

public:

    typedef T element_type;
	typedef weak_ptr<T> type;

    weak_ptr(): px(0), pn()
    {
    }

	template<class Y>
    weak_ptr( weak_ptr<Y> const & r ) : px(r.lock().get()), pn(r.pn) // never throws
    {
    }

    template<class Y>
    weak_ptr( weak_ptr<Y> && r )
    : px( r.lock().get() ), pn( r.pn ) // never throws
    {
        r.px = NULL;
    }

    // for better efficiency in the T == Y case
    weak_ptr( weak_ptr && r ): px( r.px ), pn( r.pn ) // never throws
    {
        r.px = NULL;
    }

    // for better efficiency in the T == Y case
    weak_ptr & operator=( weak_ptr && r ) // never throws
    {
        this_type( static_cast< weak_ptr && >( r ) ).swap( *this );
        return *this;
    }

	//template< typename Y >
    weak_ptr( typename Torque::shared_ptr<T>::type const & r )
    : px( r.px ), pn( r.pn ) // never throws
    {
    }

#if !defined(BOOST_MSVC) || (BOOST_MSVC >= 1300)

    template<class Y>
    weak_ptr & operator=(weak_ptr<Y> const & r) // never throws
    {
        px = r.lock().get();
        pn = r.pn;
        return *this;
    }

    template<class Y>
    weak_ptr & operator=(shared_ptr<Y> const & r) // never throws
    {
        px = r.px;
        pn = r.pn;
        return *this;
    }

#endif

    typename shared_ptr<T>::type lock() const // never throws
    {
		AnyStrongRefPtr<T>* ptr = (AnyStrongRefPtr<T>*)this->pn.get_ref();
        return shared_ptr<T>::type( *ptr );
    }

    long use_count() const // never throws
    {
        return pn.use_count();
    }

    bool expired() const // never throws
    {
        return pn.use_count() == 0;
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
        std::swap(px, other.px);
        pn.swap(other.pn);
    }

    void _internal_assign(T * px2, boost::detail::shared_count const & pn2)
    {
        px = px2;
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


    T * px;                       // contained pointer
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


