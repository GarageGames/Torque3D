#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>


typedef struct ll_ringbuffer ll_ringbuffer_t;
typedef struct ll_ringbuffer_data {
    char *buf;
    size_t len;
} ll_ringbuffer_data_t;

ll_ringbuffer_t *ll_ringbuffer_create(size_t sz, size_t elem_sz);
void ll_ringbuffer_free(ll_ringbuffer_t *rb);
void ll_ringbuffer_reset(ll_ringbuffer_t *rb);

void ll_ringbuffer_get_read_vector(const ll_ringbuffer_t *rb, ll_ringbuffer_data_t *vec);
void ll_ringbuffer_get_write_vector(const ll_ringbuffer_t *rb, ll_ringbuffer_data_t *vec);

size_t ll_ringbuffer_read(ll_ringbuffer_t *rb, char *dest, size_t cnt);
size_t ll_ringbuffer_peek(ll_ringbuffer_t *rb, char *dest, size_t cnt);
void ll_ringbuffer_read_advance(ll_ringbuffer_t *rb, size_t cnt);
size_t ll_ringbuffer_read_space(const ll_ringbuffer_t *rb);

size_t ll_ringbuffer_write(ll_ringbuffer_t *rb, const char *src, size_t cnt);
void ll_ringbuffer_write_advance(ll_ringbuffer_t *rb, size_t cnt);
size_t ll_ringbuffer_write_space(const ll_ringbuffer_t *rb);

#endif /* RINGBUFFER_H */
