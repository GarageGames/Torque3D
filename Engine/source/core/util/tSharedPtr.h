#ifndef _T3D_SHARED_PTR_H_
#define _T3D_SHARED_PTR_H_

#include "core/util/refBase.h"
#include "platform\platformAssert.h"
#include "core\util\autoPtr.h"

namespace Torque
{
	template<class T> class weak_ptr;
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
		AnyStrongRef() : AnyStrongRefBase() {}
		virtual void destroySelf() { if(_px) delete (Y*)(_px); delete this; }
		Y* get_ptr() const { return (Y*)_px; }

		template<class U>
		AnyStrongRef(AnyStrongRef<U> const &p)
		{			
			_px= (Y*)p.get_ptr();
		}
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

	/*template<typename Y>
	AnyStrongRefPtr<T>& operator=( const AnyStrongRefPtr<Y>& p )
	{
		set( p.ref.getPointer() );

		return *this;
	}*/

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


template<class T> class tSharedPtr
{
private:
   
    typedef tSharedPtr<T> this_type;

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

public:

    typedef T element_type;
    typedef T value_type;
    typedef T * pointer;
    

    tSharedPtr():  pn() // never throws in 1.30+
    {
    }

	tSharedPtr( const AnyStrongRefPtr<T> &r ): pn( r ) // never throws
    {
    } 

    template<class Y>
    tSharedPtr( Y * p ): pn( p ) // Y must be complete
    {       
    }   

	template<class Y, class D>
    tSharedPtr( Y * p, D* d ): pn( p ) // Y must be complete
    {       
    } 

    tSharedPtr( tSharedPtr const & r ): pn( r.pn ) // never throws
    {
    }  

	template<typename Y>
	tSharedPtr( tSharedPtr<Y> const & r ): pn( r.pn ) // never throws
    {
    }  

	template<class Y>
    explicit tSharedPtr(weak_ptr<Y> const & r): pn(r.pn) // may throw
    {
       
    }

	//dynamic_cast_tag
	template<class Y>
    tSharedPtr(tSharedPtr<Y> const & r, detail::dynamic_cast_tag): pn(r.pn)
    {		
		//if( !dynamic_cast<element_type*>( r.pn.ref.getPointer() ) ) // need to allocate new counter -- the cast failed
		if( !dynamic_cast<T*>( r.pn.get_real_ptr() ) )
        {
            pn = AnyStrongRefPtr<T>();
        }
		
    }

	template<class Y>
    tSharedPtr(tSharedPtr<Y> const & r, detail::static_cast_tag) 
    {
		pn = static_cast< AnyStrongRefPtr<Y> >(r.pn);		
    }
	 

    // assignment

    tSharedPtr & operator=( tSharedPtr const & r ) // never throws
    {
        this_type(r).swap(*this);
        return *this;
    }	

    template<class Y>
    tSharedPtr & operator=( AutoPtr<Y> & r )
    {
        this_type(r).swap(*this);
        return *this;
    }

	template<class Y>
    tSharedPtr & operator=( Torque::weak_ptr<Y> & r )
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

	typedef T * this_type::*unspecified_bool_type;

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

    void swap(tSharedPtr<T> & other) // never throws
    {  
        pn.swap(other.pn);
    }

	void swap(Torque::weak_ptr<T> & other) // never throws
    {    
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

    T * __px;                     // contained pointer
    AnyStrongRefPtr<T> pn;    // reference counter

};  // tSharedPtr


template<class T, class U> inline bool operator==(tSharedPtr<T> const & a, tSharedPtr<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(tSharedPtr<T> const & a, tSharedPtr<U> const & b)
{
    return a.get() != b.get();
}

template<class T, class U> tSharedPtr<T> dynamic_pointer_cast(tSharedPtr<U> const & r)
{
    return tSharedPtr<T>(r, Torque::detail::dynamic_cast_tag());
}

template<class T, class U> tSharedPtr<T> static_pointer_cast(tSharedPtr<U> const & r)
{
    return tSharedPtr<T>(r, Torque::detail::static_cast_tag());
}

template<class T, class U> inline bool operator<(tSharedPtr<T> const & a, tSharedPtr<U> const & b)
{
    return a.owner_before( b );
}

template<typename T>
struct shared_ptr
{
	static tSharedPtr<T> get_type();
	tSharedPtr<T> p;
	typedef tSharedPtr<T> type;
private:
	shared_ptr() {}
};


};

#endif 