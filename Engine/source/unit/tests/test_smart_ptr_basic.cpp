//
//  shared_ptr_basic_test.cpp
//
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "core\util\tSharedPtr.h"
#include "core\util\tWeakPtr.h"
#include "core\util\tScopedPtr.h"

#define TEST_ASSERT(b) AssertFatal(b,"")

namespace {

int cnt = 0;

struct X
{
    X()
    {
        ++cnt;
    }

    ~X() // virtual destructor deliberately omitted
    {
        --cnt;
    }

    virtual int id() const
    {
        return 1;
    }

private:

    //X(X const &);
    X & operator= (X const &);
};

struct Y: public X
{
    Y()
    {
        ++cnt;
    }

    ~Y()
    {
        --cnt;
    }

    virtual int id() const
    {
        return 2;
    }

private:

    Y(Y const &);
    Y & operator= (Y const &);
};

int * get_object()
{
    ++cnt;
    return &cnt;
}

void release_object(int * p)
{
    TEST_ASSERT(p == &cnt);
    --cnt;
}

template<class T> void test_is_X(Torque::SharedPtr<T> const & p)
{
    TEST_ASSERT(p->id() == 1);
    TEST_ASSERT((*p).id() == 1);
}

template<class T> void test_is_X(Torque::WeakPtr<T> const & p)
{
    TEST_ASSERT(p.get() != 0);
    TEST_ASSERT(p.get()->id() == 1);
}

template<class T> void test_is_Y(Torque::SharedPtr<T> const & p)
{
    TEST_ASSERT(p->id() == 2);
    TEST_ASSERT((*p).id() == 2);
}

template<class T> void test_is_Y(Torque::WeakPtr<T> const & p)
{
    Torque::SharedPtr<T> q = p.lock();
    TEST_ASSERT(q.get() != 0);
    TEST_ASSERT(q->id() == 2);
}

template<class T> void test_eq(T const & a, T const & b)
{
    TEST_ASSERT(a == b);
    TEST_ASSERT(!(a != b));
    TEST_ASSERT(!(a < b));
    TEST_ASSERT(!(b < a));
}

template<class T> void test_ne(T const & a, T const & b)
{
    TEST_ASSERT(!(a == b));
    TEST_ASSERT(a != b);
    TEST_ASSERT(a < b || b < a);
    TEST_ASSERT(!(a < b && b < a));
}

template<class T, class U> void test_shared(Torque::WeakPtr<T> const & a, Torque::WeakPtr<U> const & b)
{
    TEST_ASSERT(!(a < b));
    TEST_ASSERT(!(b < a));
}

template<class T, class U> void test_nonshared(Torque::WeakPtr<T> const & a, Torque::WeakPtr<U> const & b)
{
    TEST_ASSERT(a < b || b < a);
    TEST_ASSERT(!(a < b && b < a));
}

template<class T, class U> void test_eq2(T const & a, U const & b)
{
    TEST_ASSERT(a == b);
    TEST_ASSERT(!(a != b));
}

template<class T, class U> void test_ne2(T const & a, U const & b)
{
    TEST_ASSERT(!(a == b));
    TEST_ASSERT(a != b);
}

template<class T> void test_is_zero(Torque::SharedPtr<T> const & p)
{
    TEST_ASSERT(!p);
    TEST_ASSERT(p.get() == 0);
}

template<class T> void test_is_nonzero(Torque::SharedPtr<T> const & p)
{
    // p? true: false is used to test p in a boolean context.
    // TEST_ASSERT(p) is not guaranteed to test the conversion,
    // as the macro might test !!p instead.
    TEST_ASSERT(p? true: false);
    TEST_ASSERT(p.get() != 0);
}

};

namespace Torque{ namespace Test_Unit {

bool test_shared_ptr()
{
	TEST_ASSERT(cnt == 0);
    {		
        Torque::SharedPtr<X> p(new Y);
        Torque::SharedPtr<X> p2(new X);

        test_is_nonzero(p);
        test_is_nonzero(p2);
        test_is_Y(p);
        test_is_X(p2);
        test_ne(p, p2);

        {
            Torque::SharedPtr<X> q(p);
            test_eq(p, q);
        }
		

        Torque::SharedPtr<Y> p3 = Torque::dynamic_pointer_cast<Y>(p);
        Torque::SharedPtr<Y> p4 = Torque::dynamic_pointer_cast<Y>(p2);

        test_is_nonzero(p3);
        test_is_zero(p4);

        TEST_ASSERT(p.use_count() == 2);
        TEST_ASSERT(p2.use_count() == 1);
        TEST_ASSERT(p3.use_count() == 2);

        test_is_Y(p3);
        test_eq2(p, p3);
        test_ne2(p2, p4);

        Torque::SharedPtr<void> p5(p);

        test_is_nonzero(p5);
        test_eq2(p, p5);

        Torque::WeakPtr<X> wp1(p2);

        TEST_ASSERT(!wp1.expired());
        TEST_ASSERT(wp1.use_count() != 0);

        p.reset();
        p2.reset();

        p3.reset();
        p4.reset();


        test_is_zero(p);
        test_is_zero(p2);

        test_is_zero(p3);
        test_is_zero(p4);

        TEST_ASSERT(p5.use_count() == 1);

        TEST_ASSERT(wp1.expired());
        TEST_ASSERT(wp1.use_count() == 0);

        try
        {
            Torque::SharedPtr<X> sp1(wp1);
            AssertFatal(0,"Torque::SharedPtr<X> sp1(wp1) failed to throw");
        }
        catch(Torque::bad_weak_ptr const &)
        {
        }

        test_is_zero(wp1.lock());


        Torque::WeakPtr<X> wp2 = Torque::static_pointer_cast<X>(p5);

        TEST_ASSERT(wp2.use_count() == 1);
        test_is_Y(wp2);
        test_nonshared(wp1, wp2);

        // Scoped to not affect the subsequent use_count() tests.
        {
            Torque::SharedPtr<X> sp2(wp2);
            test_is_nonzero(wp2.lock());
        }



        Torque::WeakPtr<Y> wp3 = Torque::dynamic_pointer_cast<Y>(wp2.lock());

        TEST_ASSERT(wp3.use_count() == 1);
        test_shared(wp2, wp3);

        Torque::WeakPtr<X> wp4(wp3);

        TEST_ASSERT(wp4.use_count() == 1);
        test_shared(wp2, wp4);


        wp1 = p2;
        test_is_zero(wp1.lock());


        wp1 = p4;
        wp1 = wp3;


        wp1 = wp2;


        TEST_ASSERT(wp1.use_count() == 1);
        test_shared(wp1, wp2);

        Torque::WeakPtr<X> wp5;


        bool b1 = wp1 < wp5;
        bool b2 = wp5 < wp1;

        p5.reset();

        TEST_ASSERT(wp1.use_count() == 0);
        TEST_ASSERT(wp2.use_count() == 0);

        TEST_ASSERT(wp3.use_count() == 0);

        // Test operator< stability for std::set< Torque::WeakPtr<> >
        // Thanks to Joe Gottman for pointing this out

        TEST_ASSERT(b1 == (wp1 < wp5));
        TEST_ASSERT(b2 == (wp5 < wp1));


        {
            // note that both get_object and release_object deal with int*

            Torque::SharedPtr<void> p6(get_object(), release_object);

        }

		

    }

    TEST_ASSERT(cnt == 0);

    return 0;
}

//AutoPtr
bool test_SharedPtr_AutoPtr()
{
	TEST_ASSERT(cnt == 0);
	{
		AutoPtr<Y> auto_ptr( new Y );
		Torque::SharedPtr<void> p( auto_ptr );
		TEST_ASSERT( auto_ptr.isNull() );
	
		AutoPtr<Y> auto_ptr2( new Y );
		Torque::SharedPtr<void> p2;
		p2 = auto_ptr2;
		TEST_ASSERT( auto_ptr2.isNull() );
	}
	TEST_ASSERT(cnt == 0);
	return true;
}

bool test_ScopedPtr()
{
	TEST_ASSERT(cnt == 0);
	{
		Torque::ScopedPtr<X> sp1 ( new X );
		//Torque::ScopedPtr<X> sp2 ( new Y ); //Error	
		
		AutoPtr<Y> auto_ptr( new Y );
		Torque::ScopedPtr<Y> sp4( auto_ptr.release() );
		TEST_ASSERT( auto_ptr.isNull() );	
	}
	TEST_ASSERT(cnt == 0);
	return true;
}

}; };

#if 0

namespace {
struct module
{
	module() 
	{ 
		Torque::Test_Unit::test_shared_ptr();
		Torque::Test_Unit::test_SharedPtr_AutoPtr();
		Torque::Test_Unit::test_ScopedPtr();
	}
} module_;
};

#endif