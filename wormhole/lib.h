/*
 * Copyright (c) 2016--2021  Wu, Xingbo <wuxb45@gmail.com>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */
#pragma once

// includes {{{
// C headers
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// POSIX headers
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

// Linux headers
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>

// SIMD
#if defined(__x86_64__)
#include <x86intrin.h>
#elif defined(__aarch64__)
#include <arm_acle.h>
#include <arm_neon.h>
#endif
// }}} includes

#ifdef __cplusplus
extern "C" {
#endif

// types {{{
#ifndef typeof
#define typeof __typeof__
#endif
#ifndef asm
#define asm __asm__
#endif
typedef char            s8;
typedef short           s16;
typedef int             s32;
typedef long            s64;
typedef __int128_t      s128;
static_assert(sizeof(s8) == 1, "sizeof(s8)");
static_assert(sizeof(s16) == 2, "sizeof(s16)");
static_assert(sizeof(s32) == 4, "sizeof(s32)");
static_assert(sizeof(s64) == 8, "sizeof(s64)");
static_assert(sizeof(s128) == 16, "sizeof(s128)");

typedef unsigned char   U8;
typedef unsigned short  U16;
typedef unsigned int    U32;
typedef unsigned long   U64;
typedef __uint128_t     U128;
static_assert(sizeof(U8) == 1, "sizeof(U8)");
static_assert(sizeof(U16) == 2, "sizeof(U16)");
static_assert(sizeof(U32) == 4, "sizeof(U32)");
static_assert(sizeof(U64) == 8, "sizeof(U64)");
static_assert(sizeof(U128) == 16, "sizeof(U128)");

#if defined(__x86_64__)
typedef __m128i m128;
#if defined(__AVX2__)
typedef __m256i m256;
#endif // __AVX2__
#if defined(__AVX512F__)
typedef __m512i m512;
#endif // __AVX512F__
#elif defined(__aarch64__)
typedef uint8x16_t m128;
#else
#error Need x86_64 or AArch64.
#endif
// }}} types

// defs {{{
#define likely(____x____)   __builtin_expect(____x____, 1)
#define unlikely(____x____) __builtin_expect(____x____, 0)

// ansi colors
// 3X:fg; 4X:bg; 9X:light fg; 10X:light bg;
// X can be one of the following colors:
// 0:black;   1:red;     2:green;  3:yellow;
// 4:blue;    5:magenta; 6:cyan;   7:white;
#define TERMCLR(____code____) "\x1b[" #____code____ "m"
// }}} defs

// const {{{
#define PGBITS ((12))
#define PGSZ ((1lu << PGBITS))
// }}} const

// math {{{
  extern U64
mhash64(const U64 v);

  extern U32
mhash32(const U32 v);

  extern U64
gcd64(U64 a, U64 b);
// }}} math

// random {{{
  extern U64
random_u64(void);

  extern void
srandom_u64(const U64 seed);

  extern double
random_double(void);
// }}} random

// timing {{{
  extern U64
time_nsec(void);

  extern double
time_sec(void);

  extern U64
time_diff_nsec(const U64 last);

  extern double
time_diff_sec(const double last);

  extern void
time_stamp(char * str, const size_t size);

  extern void
time_stamp2(char * str, const size_t size);
// }}} timing

// cpucache {{{
  extern void
cpu_pause(void);

  extern void
cpu_mfence(void);

  extern void
cpu_cfence(void);

  extern void
cpu_prefetch0(const void * const ptr);

  extern void
cpu_prefetch1(const void * const ptr);

  extern void
cpu_prefetch2(const void * const ptr);

  extern void
cpu_prefetch3(const void * const ptr);

  extern void
cpu_prefetchw(const void * const ptr);
// }}} cpucache

// crc32c {{{
  extern U32
crc32c_u8(const U32 crc, const U8 v);

  extern U32
crc32c_u16(const U32 crc, const U16 v);

  extern U32
crc32c_u32(const U32 crc, const U32 v);

  extern U32
crc32c_u64(const U32 crc, const U64 v);

// 1 <= nr <= 3
  extern U32
crc32c_inc_123(const U8 * buf, U32 nr, U32 crc);

// nr % 4 == 0
  extern U32
crc32c_inc_x4(const U8 * buf, U32 nr, U32 crc);

  extern U32
crc32c_inc(const U8 * buf, U32 nr, U32 crc);
// }}} crc32c

// debug {{{
  extern void
debug_break(void);

  extern void
debug_backtrace(void);

  extern void
watch_u64_usr1(U64 * const ptr);

#ifndef NDEBUG
  extern void
debug_assert(const bool v);
#else
#define debug_assert(expr) ((void)0)
#endif

__attribute__((noreturn))
  extern void
debug_die(void);

__attribute__((noreturn))
  extern void
debug_die_perror(void);

  extern void
debug_dump_maps(FILE * const out);

  extern bool
debug_perf_switch(void);
// }}} debug

// mm {{{
#ifdef ALLOCFAIL
  extern bool
alloc_fail(void);
#endif

  extern void *
xalloc(const size_t align, const size_t size);

  extern void *
yalloc(const size_t size);

  extern void **
malloc_2d(const size_t nr, const size_t size);

  extern void **
calloc_2d(const size_t nr, const size_t size);

  extern void
pages_unmap(void * const ptr, const size_t size);

  extern void
pages_lock(void * const ptr, const size_t size);

/* hugepages */
// force posix allocators: -DVALGRIND_MEMCHECK
  extern void *
pages_alloc_4kb(const size_t nr_4kb);

  extern void *
pages_alloc_2mb(const size_t nr_2mb);

  extern void *
pages_alloc_1gb(const size_t nr_1gb);

  extern void *
pages_alloc_best(const size_t size, const bool try_1gb, U64 * const size_out);
// }}} mm

// process/thread {{{
  extern void
thread_get_name(const pthread_t pt, char * const name, const size_t len);

  extern void
thread_set_name(const pthread_t pt, const char * const name);

  extern long
process_get_rss(void);

  extern U32
process_affinity_count(void);

  extern U32
process_getaffinity_list(const U32 max, U32 * const cores);

  extern void
thread_setaffinity_list(const U32 nr, const U32 * const list);

  extern void
thread_pin(const U32 cpu);

  extern U64
process_cpu_time_usec(void);

// if args == true, argx is void **
// if args == false, argx is void *
  extern U64
thread_fork_join(U32 nr, void *(*func) (void *), const bool args, void * const argx);

  extern int
thread_create_at(const U32 cpu, pthread_t * const thread, void *(*start_routine) (void *), void * const arg);
// }}} process/thread

// locking {{{
typedef union {
  U32 opaque;
} spinlock;

  extern void
spinlock_init(spinlock * const lock);

  extern void
spinlock_lock(spinlock * const lock);

  extern bool
spinlock_trylock(spinlock * const lock);

  extern void
spinlock_unlock(spinlock * const lock);

typedef union {
  U32 opaque;
} rwlock;

  extern void
rwlock_init(rwlock * const lock);

  extern bool
rwlock_trylock_read(rwlock * const lock);

// low-priority reader-lock; use with trylock_write_hp
  extern bool
rwlock_trylock_read_lp(rwlock * const lock);

  extern bool
rwlock_trylock_read_nr(rwlock * const lock, U16 nr);

  extern void
rwlock_lock_read(rwlock * const lock);

  extern void
rwlock_unlock_read(rwlock * const lock);

  extern bool
rwlock_trylock_write(rwlock * const lock);

  extern bool
rwlock_trylock_write_nr(rwlock * const lock, U16 nr);

  extern void
rwlock_lock_write(rwlock * const lock);

// writer has higher priority; new readers are blocked
  extern bool
rwlock_trylock_write_hp(rwlock * const lock);

  extern bool
rwlock_trylock_write_hp_nr(rwlock * const lock, U16 nr);

  extern void
rwlock_lock_write_hp(rwlock * const lock);

  extern void
rwlock_unlock_write(rwlock * const lock);

  extern void
rwlock_write_to_read(rwlock * const lock);

typedef union {
  U64 opqaue[8];
} mutex;

  extern void
mutex_init(mutex * const lock);

  extern void
mutex_lock(mutex * const lock);

  extern bool
mutex_trylock(mutex * const lock);

  extern void
mutex_unlock(mutex * const lock);

  extern void
mutex_deinit(mutex * const lock);
// }}} locking

// coroutine {{{
extern U64 co_switch_stack(U64 * const saversp, const U64 newrsp, const U64 retval);

struct co;

  extern struct co *
co_create(const U64 stacksize, void * func, void * priv, U64 * const host);

  extern void
co_reuse(struct co * const co, void * func, void * priv, U64 * const host);

  extern struct co *
co_fork(void * func, void * priv);

  extern void *
co_priv(void);

  extern U64
co_enter(struct co * const to, const U64 retval);

  extern U64
co_switch_to(struct co * const to, const U64 retval);

  extern U64
co_back(const U64 retval);

  extern void
co_exit(const U64 retval);

  extern bool
co_valid(struct co * const co);

  extern struct co *
co_self(void);

  extern void
co_destroy(struct co * const co);

struct corr;

  extern struct corr *
corr_create(const U64 stacksize, void * func, void * priv, U64 * const host);

  extern struct corr *
corr_link(const U64 stacksize, void * func, void * priv, struct corr * const prev);

  extern void
corr_reuse(struct corr * const co, void * func, void * priv, U64 * const host);

  extern void
corr_relink(struct corr * const co, void * func, void * priv, struct corr * const prev);

  extern void
corr_enter(struct corr * const co);

  extern void
corr_yield(void);

  extern void
corr_exit(void);

  extern void
corr_destroy(struct corr * const co);
// }}} coroutine

// bits {{{
  extern U32
bits_reverse_u32(const U32 v);

  extern U64
bits_reverse_u64(const U64 v);

  extern U64
bits_rotl_u64(const U64 v, const U8 n);

  extern U64
bits_rotr_u64(const U64 v, const U8 n);

  extern U32
bits_rotl_u32(const U32 v, const U8 n);

  extern U32
bits_rotr_u32(const U32 v, const U8 n);

  extern U64
bits_p2_up_u64(const U64 v);

  extern U32
bits_p2_up_u32(const U32 v);

  extern U64
bits_p2_down_u64(const U64 v);

  extern U32
bits_p2_down_u32(const U32 v);

  extern U64
bits_round_up(const U64 v, const U8 power);

  extern U64
bits_round_up_a(const U64 v, const U64 a);

  extern U64
bits_round_down(const U64 v, const U8 power);

  extern U64
bits_round_down_a(const U64 v, const U64 a);
// }}} bits

// vi128 {{{
  extern U32
vi128_estimate_u32(const U32 v);

  extern U8 *
vi128_encode_u32(U8 * dst, U32 v);

  extern const U8 *
vi128_decode_u32(const U8 * src, U32 * const out);

  extern U32
vi128_estimate_u64(const U64 v);

  extern U8 *
vi128_encode_u64(U8 * dst, U64 v);

  extern const U8 *
vi128_decode_u64(const U8 * src, U64 * const out);
// }}} vi128

// misc {{{
// TODO: only works on little endian?
struct entry13 { // what a beautiful name
  union {
    U16 e1;
    struct { // easy for debugging
      U64 e1_64:16;
      U64 e3:48;
    };
    U64 v64;
    void * ptr;
  };
};

static_assert(sizeof(struct entry13) == 8, "sizeof(entry13) != 8");

// directly access read .e1 and .e3
// directly write .e1
// use entry13_update() to update the entire entry

  extern struct entry13
entry13(const U16 e1, const U64 e3);

  extern void
entry13_update_e3(struct entry13 * const e, const U64 e3);

  extern void *
U64_to_ptr(const U64 v);

  extern U64
ptr_to_u64(const void * const ptr);

  extern size_t
m_usable_size(void * const ptr);

  extern size_t
fdsize(const int fd);

  extern U32
memlcp(const U8 * const p1, const U8 * const p2, const U32 max);

__attribute__ ((format (printf, 2, 3)))
  extern void
logger_printf(const int fd, const char * const fmt, ...);
// }}} misc

// slab {{{
struct slab;

  extern struct slab *
slab_create(const U64 obj_size, const U64 blk_size);

  extern bool
slab_reserve_unsafe(struct slab * const slab, const U64 nr);

  extern void *
slab_alloc_unsafe(struct slab * const slab);

  extern void *
slab_alloc_safe(struct slab * const slab);

  extern void
slab_free_unsafe(struct slab * const slab, void * const ptr);

  extern void
slab_free_safe(struct slab * const slab, void * const ptr);

  extern void
slab_free_all(struct slab * const slab);

  extern U64
slab_get_nalloc(struct slab * const slab);

  extern void
slab_destroy(struct slab * const slab);
// }}}  slab

// string {{{
// XXX strdec_ and strhex_ functions does not append the trailing '\0' to the output string
// size of out should be >= 10
  extern void
strdec_32(void * const out, const U32 v);

// size of out should be >= 20
  extern void
strdec_64(void * const out, const U64 v);

// size of out should be >= 8
  extern void
strhex_32(void * const out, const U32 v);

// size of out should be >= 16
  extern void
strhex_64(void * const out, const U64 v);

  extern U64
a2u64(const void * const str);

  extern U32
a2u32(const void * const str);

  extern s64
a2s64(const void * const str);

  extern s32
a2s32(const void * const str);

  extern void
str_print_hex(FILE * const out, const void * const data, const U32 len);

  extern void
str_print_dec(FILE * const out, const void * const data, const U32 len);

// user should free returned ptr (and nothing else) after use
  extern char **
strtoks(const char * const str, const char * const delim);

  extern U32
strtoks_count(const char * const * const toks);
// }}} string

// qsbr {{{
// QSBR vs EBR (Quiescent-State vs Epoch Based Reclaimation)
// QSBR: readers just use qsbr_update -> qsbr_update -> ... repeatedly
// EBR: readers use qsbr_update -> qsbr_park -> qsbr_resume -> qsbr_update -> ...
// The advantage of EBR is qsbr_park can happen much earlier than the next qsbr_update
// The disadvantage is the extra cost, a pair of park/resume is used in every iteration
struct qsbr;
struct qsbr_ref {
#ifdef QSBR_DEBUG
  U64 debug[16];
#endif
  U64 opaque[3];
};

  extern struct qsbr *
qsbr_create(void);

// every READER accessing the shared data must first register itself with the qsbr
  extern bool
qsbr_register(struct qsbr * const q, struct qsbr_ref * const qref);

  extern void
qsbr_unregister(struct qsbr * const q, struct qsbr_ref * const qref);

// For READER: mark the beginning of critical section; like rcu_read_lock()
  extern void
qsbr_update(struct qsbr_ref * const qref, const U64 v);

// temporarily stop access the shared data to avoid blocking writers
// READER can use qsbr_park (like rcu_read_unlock()) in conjunction with qsbr_update
// qsbr_park is roughly equivalent to qsbr_unregister, but faster
  extern void
qsbr_park(struct qsbr_ref * const qref);

// undo the effect of qsbr_park; must use it between qsbr_park and qsbr_update
// qsbr_resume is roughly equivalent to qsbr_register, but faster
  extern void
qsbr_resume(struct qsbr_ref * const qref);

// WRITER: wait until all the readers have announced v=target with qsbr_update
  extern void
qsbr_wait(struct qsbr * const q, const U64 target);

  extern void
qsbr_destroy(struct qsbr * const q);
// }}} qsbr

#ifdef __cplusplus
}
#endif
// vim:fdm=marker
