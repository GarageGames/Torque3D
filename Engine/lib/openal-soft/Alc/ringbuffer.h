#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct ll_ringbuffer ll_ringbuffer_t;
typedef struct ll_ringbuffer_data {
    char *buf;
    size_t len;
} ll_ringbuffer_data_t;


/**
 * Create a new ringbuffer to hold at least `sz' elements of `elem_sz' bytes.
 * The number of elements is rounded up to the next power of two (even if it is
 * already a power of two, to ensure the requested amount can be written).
 */
ll_ringbuffer_t *ll_ringbuffer_create(size_t sz, size_t elem_sz, int limit_writes);
/** Free all data associated with the ringbuffer `rb'. */
void ll_ringbuffer_free(ll_ringbuffer_t *rb);
/** Reset the read and write pointers to zero. This is not thread safe. */
void ll_ringbuffer_reset(ll_ringbuffer_t *rb);

/**
 * The non-copying data reader. `vec' is an array of two places. Set the values
 * at `vec' to hold the current readable data at `rb'. If the readable data is
 * in one segment the second segment has zero length.
 */
void ll_ringbuffer_get_read_vector(const ll_ringbuffer_t *rb, ll_ringbuffer_data_t vec[2]);
/**
 * The non-copying data writer. `vec' is an array of two places. Set the values
 * at `vec' to hold the current writeable data at `rb'. If the writeable data
 * is in one segment the second segment has zero length.
 */
void ll_ringbuffer_get_write_vector(const ll_ringbuffer_t *rb, ll_ringbuffer_data_t vec[2]);

/**
 * Return the number of elements available for reading. This is the number of
 * elements in front of the read pointer and behind the write pointer.
 */
size_t ll_ringbuffer_read_space(const ll_ringbuffer_t *rb);
/**
 * The copying data reader. Copy at most `cnt' elements from `rb' to `dest'.
 * Returns the actual number of elements copied.
 */
size_t ll_ringbuffer_read(ll_ringbuffer_t *rb, char *dest, size_t cnt);
/**
 * The copying data reader w/o read pointer advance. Copy at most `cnt'
 * elements from `rb' to `dest'. Returns the actual number of elements copied.
 */
size_t ll_ringbuffer_peek(ll_ringbuffer_t *rb, char *dest, size_t cnt);
/** Advance the read pointer `cnt' places. */
void ll_ringbuffer_read_advance(ll_ringbuffer_t *rb, size_t cnt);

/**
 * Return the number of elements available for writing. This is the number of
 * elements in front of the write pointer and behind the read pointer.
 */
size_t ll_ringbuffer_write_space(const ll_ringbuffer_t *rb);
/**
 * The copying data writer. Copy at most `cnt' elements to `rb' from `src'.
 * Returns the actual number of elements copied.
 */
size_t ll_ringbuffer_write(ll_ringbuffer_t *rb, const char *src, size_t cnt);
/** Advance the write pointer `cnt' places. */
void ll_ringbuffer_write_advance(ll_ringbuffer_t *rb, size_t cnt);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RINGBUFFER_H */
