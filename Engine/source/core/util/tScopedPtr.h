#ifndef _T3T_SCOPED_PTR_H_
#define _T3T_SCOPED_PTR_H_

//
//  Copyright (c) 2001, 2002, 2003 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/WeakPtr.htm for documentation.
//

#include "platform\platformAssert.h"

namespace Torque
{
	template<class T> 
	class ScopedPtr // noncopyable
	{
	private:

		T * px;

		ScopedPtr(ScopedPtr const &);
		ScopedPtr & operator=(ScopedPtr const &);

		typedef ScopedPtr<T> this_type;

		void operator==( ScopedPtr const& ) const;
		void operator!=( ScopedPtr const& ) const;

	public:

		typedef T element_type;

		typedef ScopedPtr<T> type;

		explicit ScopedPtr( T * p = 0 ): px( p ) // never throws
		{	
		}	

		//Not valid
		template< class Y>
		explicit ScopedPtr( Y * p ): px( p ) // never throws
		{	
			Y* _y = px; //Forced error
		}	

		~ScopedPtr() // never throws
		{
			if( px )
				delete px;
		}

		void reset(T * p = 0) // never throws
		{
			AssertFatal( p == 0 || p != px ,""); // catch self-reset errors
			this_type(p).swap(*this);
		}

		T & operator*() const // never throws
		{
			AssertFatal( px != 0, "");
			return *px;
		}

		T * operator->() const // never throws
		{
			AssertFatal( px != 0, "");
			return px;
		}

		T * get() const // never throws
		{
			return px;
		}

		// implicit conversion to "bool"
		typedef T * this_type::*unspecified_bool_type;

		operator unspecified_bool_type() const // never throws
		{
			return px == 0? 0: &this_type::px;
		}

		void swap(ScopedPtr & b) // never throws
		{
			T * tmp = b.px;
			b.px = px;
			px = tmp;
		}
	};
	
};

	

template<class T>
inline void swap( Torque::ScopedPtr<T> & a, Torque::ScopedPtr<T> & b) // never throws
{
    a.swap(b);
}

// get_pointer(p) is a generic way to say p.get()

template<class T>
inline T * get_pointer( Torque::ScopedPtr<T> const & p)
{
    return p.get();
}

#endif