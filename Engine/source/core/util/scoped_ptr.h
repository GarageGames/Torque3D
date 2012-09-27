#ifndef _T3T_SCOPED_PTR_H_

#include "core\util\autoPtr.h"
#include "platform\platformAssert.h"

namespace Torque
{
	template<class T> 
	class tScopedPtr // noncopyable
	{
	private:

		T * px;

		tScopedPtr(tScopedPtr const &);
		tScopedPtr & operator=(tScopedPtr const &);

		typedef tScopedPtr<T> this_type;

		void operator==( tScopedPtr const& ) const;
		void operator!=( tScopedPtr const& ) const;

	public:

		typedef T element_type;

		typedef tScopedPtr<T> type;

		explicit tScopedPtr( T * p = 0 ): px( p ) // never throws
		{	
		}

		explicit tScopedPtr( AutoPtr<T> p ): px( p.release() ) // never throws
		{
		}

		~tScopedPtr() // never throws
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

		void swap(tScopedPtr & b) // never throws
		{
			T * tmp = b.px;
			b.px = px;
			px = tmp;
		}
	};

	template<typename T>
	struct scoped_ptr
	{
		typedef tScopedPtr<T> type;
	private:
		scoped_ptr() {}
	};
	
};

	

template<class T>
inline void swap(typename Torque::scoped_ptr<T>::type & a, typename Torque::scoped_ptr<T>::type & b) // never throws
{
    a.swap(b);
}

// get_pointer(p) is a generic way to say p.get()

template<class T>
inline T * get_pointer(typename Torque::scoped_ptr<T>::type const & p)
{
    return p.get();
}

#endif