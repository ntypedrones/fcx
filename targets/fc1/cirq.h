/* Copyright 2016 Julian Ingram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CIRQ_H
#define CIRQ_H

#include <stdlib.h>
#include <string.h>

struct cirq
{
    volatile unsigned char* buffer;
    volatile unsigned char* buffer_limit;
    volatile unsigned char* buffer_max;
    volatile unsigned char* volatile head;
    volatile unsigned char* volatile tail;
    size_t element_size;
};

static inline struct cirq cirq_init(volatile void* buffer,
    const size_t width, const size_t element_size)
{
    volatile unsigned char* limit = ((volatile unsigned char*)buffer) + (width *
        element_size);
    struct cirq cirq = {
        .buffer = buffer,
        .buffer_limit = limit,
        .buffer_max = limit - element_size,
        .head = buffer,
        .tail = buffer,
        .element_size = element_size,
    };
    return cirq;
}

static inline char cirq_empty(const struct cirq* const c)
{
    return (c->head == c->tail) ? 1 : 0;
}

// This space function returns one less than the actual space available, because
// when the buffer is (actually) full, head == tail and it is indistinguishable
// from an empty buffer. This will not be efficient for larger element sizes.

static inline size_t cirq_space(const struct cirq* const c)
{
    return (c->tail >= c->head) ? ((size_t)((c->buffer_limit - c->tail) +
	    (c->head - c->buffer)) / c->element_size) - 1u :
        ((size_t)(c->head - c->tail) / c->element_size) - 1u;
}

static inline size_t cirq_population(const struct cirq* const c)
{
    return (c->tail >= c->head) ? ((size_t)(c->tail - c->head) /
        c->element_size) : ((size_t)((c->buffer_limit - c->head) + (c->tail -
        c->buffer)) / c->element_size);
}

static inline void cirq_flush(struct cirq* const c)
{
    c->tail = c->head;
}

// tail always points to free space, head points to a given value, except when
// the queue is empty.

static inline volatile unsigned char* cirq_dec(const struct cirq* const c,
    volatile unsigned char* const p)
{
    return (p == c->buffer) ? c->buffer_max : p - c->element_size;
}

static inline volatile unsigned char* cirq_inc(const struct cirq* const c,
    volatile unsigned char* const p)
{
    return (p == c->buffer_max) ? c->buffer : p + c->element_size;
}

static inline void cirq_vol_memcpy(volatile void* const dst,
    const volatile void* const src, const size_t size)
{
    volatile unsigned char* d = (volatile unsigned char*)dst;
    const volatile unsigned char* s = (const volatile unsigned char*)src;
    volatile unsigned char* const l = d + size;
    while (d < l)
    {
        *d = *s;
        ++d;
        ++s;
    }
}

// increments tail

static inline void cirq_push_back(struct cirq* const c,
    const volatile void* const item)
{
    volatile unsigned char* const p = c->tail;
    cirq_vol_memcpy(p, item, c->element_size);
    c->tail = cirq_inc(c, p);
}

// decrements head

static inline void cirq_push_front(struct cirq* const c,
    const volatile void* const item)
{
    volatile unsigned char* const p = cirq_dec(c, c->head);
    cirq_vol_memcpy(p, item, c->element_size);
    c->head = p;
}

// decremenets tail

static inline void cirq_pop_back(struct cirq* const c,
    volatile void* const item)
{
    volatile unsigned char* const p = cirq_dec(c, c->tail);
    cirq_vol_memcpy(item, p, c->element_size);
    c->tail = p;
}

// increments head

static inline void cirq_pop_front(struct cirq* const c,
    volatile void* const item)
{
    volatile unsigned char* const p = c->head;
    cirq_vol_memcpy(item, p, c->element_size);
    c->head = cirq_inc(c, p);
}

static inline volatile unsigned char*
cirq_back_ix_to_ptr(const struct cirq* const c, size_t index)
{
    volatile unsigned char* const tail = c->tail;
    index *= c->element_size;
    if (tail == c->buffer)
    {
        return c->buffer_max - index;
    }
    volatile unsigned char* const f = tail - c->element_size;
    size_t offset = (size_t)(f - c->buffer);
    return (offset >= index) ? f - index : c->buffer_limit - (index - offset);
}

static inline volatile unsigned char*
cirq_front_ix_to_ptr(const struct cirq* const c, size_t index)
{
    volatile unsigned char* const head = c->head;
    size_t offset = (size_t)(c->buffer_limit - head);
    index *= c->element_size;
    return (offset > index) ? head + index : c->buffer + (index - offset);
}

static inline void cirq_peek_back(const struct cirq* const c,
    const size_t index, volatile void* const item)
{
    cirq_vol_memcpy(item, cirq_back_ix_to_ptr(c, index), c->element_size);
}

static inline void cirq_peek_front(const struct cirq* const c,
    const size_t index, volatile void* const item)
{
    cirq_vol_memcpy(item, cirq_front_ix_to_ptr(c, index), c->element_size);
}

static inline void cirq_place_back(struct cirq* const c, const size_t index,
    const volatile void* const item)
{
    cirq_vol_memcpy(cirq_back_ix_to_ptr(c, index), item, c->element_size);
}

static inline void cirq_place_front(struct cirq* const c, const size_t index,
    const volatile void* const item)
{
    cirq_vol_memcpy(cirq_front_ix_to_ptr(c, index), item, c->element_size);
}

#endif
