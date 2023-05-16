#ifndef PTHREAD_H
#define PTHREAD_H

#include <stdint.h>

/**
 * Partial implementation of pthreads in xv6
 * 
 * Bee Cerasoli, Ben Graham, Corwin Jones
*/

// for now the stack size per thread is fixed
// later we can implement pthread_attr_setstacksize etc
#define PTH_STACK_SIZE 2*1024*1024 // default 1MB stacks

typedef int pthread_t;

typedef uint64_t pthread_attr_t;

extern int pthread_create(pthread_t *restrict thread,
                          const pthread_attr_t *restrict attr,
                          void *(*start_routine)(void*),
                          void *restrict arg);

extern int pthread_join(pthread_t thread, void** retval);

#endif