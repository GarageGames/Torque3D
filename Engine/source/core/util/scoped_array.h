#ifndef _T3D_SCOPED_ARRAY_H_

#include "platform\platformAssert.h"

namespace Torque
{

template<class T> 
class tScopedArray // noncopyable
{
private:

    T * px;

    tScopedArray(tScopedArray const &);
    tScopedArray & operator=(tScopedArray const &);

    typedef tScopedArray<T> this_type;

    void operator==( tScopedArray const& ) const;
    void operator!=( tScopedArray const& ) const;

public:

    typedef T element_type;

	typedef tScopedArray<T> type;

    explicit tScopedArray( T * p = 0 ) : px( p ) // never throws
    {
    }

    ~tScopedArray() // never throws
    {
		if(px)
			delete [] px;
    }

    void reset(T * p = 0) // never throws
    {
        AssertFatal( p == 0 || p != px ,""); // catch self-reset errors
        this_type(p).swap(*this);
    }

    T & operator[](unsigned int i) const // never throws
    {
        AssertFatal( px != 0 ,"");
        AssertFatal( i >= 0 ,"");
        return px[i];
    }

    T * get() const // never throws
    {
        return px;
    }

	typedef T * this_type::*unspecified_bool_type;

    operator unspecified_bool_type() const // never throws
    {
        return px == 0? 0: &this_type::px;
    }

    void swap(tScopedArray & b) // never throws
    {
        T * tmp = b.px;
        b.px = px;
        px = tmp;
    }

	
};

template<typename T>
struct scoped_array
{
public:
	typedef tScopedArray<T> type;
};


template<class T> inline void swap( typename scoped_array<T>::type & a, typename scoped_array<T>::type & b) // never throws
{
    a.swap(b);
}

};


#endif 