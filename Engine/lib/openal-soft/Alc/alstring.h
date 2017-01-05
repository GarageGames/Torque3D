#ifndef ALSTRING_H
#define ALSTRING_H

#include <string.h>

#include "vector.h"


typedef char al_string_char_type;
TYPEDEF_VECTOR(al_string_char_type, al_string)
TYPEDEF_VECTOR(al_string, vector_al_string)

inline void al_string_deinit(al_string *str)
{ VECTOR_DEINIT(*str); }
#define AL_STRING_INIT(_x)       do { (_x) = (al_string)NULL; } while(0)
#define AL_STRING_INIT_STATIC()  ((al_string)NULL)
#define AL_STRING_DEINIT(_x)     al_string_deinit(&(_x))

inline size_t al_string_length(const_al_string str)
{ return VECTOR_SIZE(str); }

inline ALboolean al_string_empty(const_al_string str)
{ return al_string_length(str) == 0; }

inline const al_string_char_type *al_string_get_cstr(const_al_string str)
{ return str ? &VECTOR_FRONT(str) : ""; }

void al_string_clear(al_string *str);

int al_string_cmp(const_al_string str1, const_al_string str2);
int al_string_cmp_cstr(const_al_string str1, const al_string_char_type *str2);

void al_string_copy(al_string *str, const_al_string from);
void al_string_copy_cstr(al_string *str, const al_string_char_type *from);
void al_string_copy_range(al_string *str, const al_string_char_type *from, const al_string_char_type *to);

void al_string_append_char(al_string *str, const al_string_char_type c);
void al_string_append_cstr(al_string *str, const al_string_char_type *from);
void al_string_append_range(al_string *str, const al_string_char_type *from, const al_string_char_type *to);

#ifdef _WIN32
#include <wchar.h>
/* Windows-only methods to deal with WideChar strings. */
void al_string_copy_wcstr(al_string *str, const wchar_t *from);
void al_string_append_wcstr(al_string *str, const wchar_t *from);
void al_string_append_wrange(al_string *str, const wchar_t *from, const wchar_t *to);
#endif

#endif /* ALSTRING_H */
