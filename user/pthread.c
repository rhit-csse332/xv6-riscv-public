#include "kernel/types.h"
#include "kernel/clone.h"
#include "pthread.h"
#include <user/user.h>

// constants, can't import normal POSIX headers because of naming conflicts with xv6
#define SIGCHLD 17
#define CLONE_VM 0x00000100
#define EAGAIN 11

struct pthread_internal_args {
    void *(*fn)(void*);
    void *arg;
};

int pthread_run(void* args_ptr) {
    struct pthread_internal_args args = *(struct pthread_internal_args*) args_ptr;
    free(args_ptr);
    // void* returnval = 
    args.fn(args.arg);
    exit(0);
}

int pthread_create(pthread_t *restrict thread,
                   const pthread_attr_t *restrict attr,
                   void *(*start_routine)(void*),
                   void *restrict arg) { 
    void* stack = malloc(PTH_STACK_SIZE);
    if(!stack) {
        return EAGAIN; // if we don't have stack, return error
    }

    struct pthread_internal_args *intargs = malloc(sizeof(struct pthread_internal_args));
    intargs->fn = start_routine;
    intargs->arg = arg;
    *thread = clone(pthread_run, stack+PTH_STACK_SIZE, SIGCHLD | CLONE_VM, intargs);
    if(*thread < 0) {
       free(intargs);
       free(stack);
        return *thread; // on error return code
    } else {
        return 0;
    }
}

int pthread_join(pthread_t thread, void** retval) {
    int status;
    // scuffed alternative to waitpid
    while(wait(&status) != thread);
    // retval not handled
    return 0;
}



