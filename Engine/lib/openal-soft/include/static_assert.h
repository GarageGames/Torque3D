#ifndef AL_STATIC_ASSERT_H
#define AL_STATIC_ASSERT_H

#include <assert.h>


#ifndef static_assert
#ifdef HAVE_C11_STATIC_ASSERT
#define static_assert _Static_assert
#else
#define CTASTR2(_pre,_post) _pre##_post
#define CTASTR(_pre,_post) CTASTR2(_pre,_post)
#if defined(__COUNTER__)
#define static_assert(_cond, _msg) typedef struct { int CTASTR(static_assert_failed_at_line_,__LINE__) : !!(_cond); } CTASTR(static_assertion_,__COUNTER__)
#else
#define static_assert(_cond, _msg) struct { int CTASTR(static_assert_failed_at_line_,__LINE__) : !!(_cond); }
#endif
#endif
#endif

#endif /* AL_STATIC_ASSERT_H */
