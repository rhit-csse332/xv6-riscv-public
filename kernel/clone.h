#ifndef CLONE_H
#define CLONE_H

/**
 * Kernel threads library
 * 
 * Bee Cerasoli, Ben Graham, Corwin Jones
*/

// Based on the clone() man page
//extern int clone(int (*fn)(void*), void* stack, int flags, void* arg);
int clone(int (*fn)(void*), void* stack, int flags, void* arg);

#endif