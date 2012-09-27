#ifndef _T3D_SHARED_PTR_H_
#define _T3D_SHARED_PTR_H_

#include "core/util/refBase.h"
#include "platform\platformAssert.h"
#include "core\util\autoPtr.h"

namespace Torque
{

template<typename T>
class AnyStrongRefPtr
{	
	T* px;
	StrongRefBase *ref;
public:	

	~AnyStrongRefPtr()
	{
		if(ref)	
		{
			if( ref->getRefCount() == 1)
				delete px;
			ref->decRefCount();					
		}
	}

	AnyStrongRefPtr() : px(nullptr), ref(nullptr) {}

	template<typename Y>
	explicit AnyStrongRefPtr(Y* p) : px(p)
	{
		ref = new StrongRefBase();
		ref->incRefCount();
	}

	AnyStrongRefPtr(const AnyStrongRefPtr<T>& p)
	{		
		ref = p.ref;		
		px = p.px;

		if(ref)
			ref->incRefCount();
	}

	void swap( AnyStrongRefPtr<T>& p)
	{
		AnyStrongRefPtr<T> temp = *this;
		*this = p;
		p = temp;
	}

	AnyStrongRefPtr<T>& operator=( const AnyStrongRefPtr<T>& p )
	{
		if(ref)
		{
			if( ref->getRefCount() == 1)
				delete px;
			ref->decRefCount();
		}

		ref = p.ref;		
		px = p.px;

		if(ref)
			ref->incRefCount();

		return *this;
	}



protected:
	//char buffer[ sizeof(AnyStrongRefPtr<T>) ];
};


template<class T> class tSharedPtr
{
private:
   
    typedef tSharedPtr<T> this_type;

public:

    typedef T element_type;
    typedef T value_type;
    typedef T * pointer;
    

    tSharedPtr(): px(0), pn() // never throws in 1.30+
    {
    }

    template<class Y>
    explicit tSharedPtr( Y * p ): px( p ), pn( p ) // Y must be complete
    {       
    }   

    tSharedPtr( tSharedPtr const & r ): px( r.px ), pn( r.pn ) // never throws
    {
    }    

    // assignment

    tSharedPtr & operator=( tSharedPtr const & r ) // never throws
    {
        this_type(r).swap(*this);
        return *this;
    }

	/*
#if !defined(BOOST_MSVC) || (BOOST_MSVC >= 1400)

    template<class Y>
    tSharedPtr & operator=(tSharedPtr<Y> const & r) // never throws
    {
        this_type(r).swap(*this);
        return *this;
    }

#endif
	*/

    template<class Y>
    tSharedPtr & operator=( AutoPtr<Y> & r )
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

    template<class Y> void reset( tSharedPtr<Y> const & r, T * p )
    {
        this_type( r, p ).swap( *this );
    }

    T operator* () const // never throws
    {
        AssertFatal(px != 0, "");
        return *px;
    }

    T * operator-> () const // never throws
    {
        AssertFatal(px != 0,"");
        return px;
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

    bool unique() const // never throws
    {
        return pn.unique();
    }

    long use_count() const // never throws
    {
        return pn.use_count();
    }

    void swap(tSharedPtr<T> & other) // never throws
    {        
		T* temp = px;
		px   = other.px;
		other.px   = temp;

        pn.swap(other.pn);
    }

    template<class Y> bool owner_before( tSharedPtr<Y> const & rhs ) const
    {
        return pn < rhs.pn;
    }

    bool _internal_equiv( tSharedPtr const & r ) const
    {
        return px == r.px && pn == r.pn;
    }

    T * px;                     // contained pointer
    AnyStrongRefPtr<T> pn;    // reference counter

};  // tSharedPtr

template<typename T>
struct shared_ptr
{
	typedef tSharedPtr<T> type;
private:
	shared_ptr() {}
};

};

#endif 