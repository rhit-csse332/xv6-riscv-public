#include <stdio.h>
#include <stdlib.h>

#include "pthread.h"

#define SIZEOFTHING 1000000

void* count(void* countptr) {
    int count = *(int*) countptr;
    for(int i = 0; i < count; i++) {
        printf("%i...\n", i+1);
        sleep(1);
    }
}

void* pokemem(void* mem) { // test access to memory
    char* memchar = mem;
    sleep(1);
    printf("hi\n");
    for(int i = 0; i < SIZEOFTHING; i++) {
        memchar[i] = 'h';
    }
    printf("yo\n");
}

void* testret(void* whatever) {
    sleep(1);
    int* ret = malloc(sizeof(int));
    *ret = 1234;
    return(ret);
}

int main(int argc, int argv) {
    pthread_t thread;
    int othercount = 5;
    int mycount = 3;

    pthread_create(&thread, 0, count, &othercount);
    count(&mycount);
    pthread_join(thread, NULL);

    char* mem = malloc(SIZEOFTHING);
    pthread_create(&thread, 0, pokemem, mem);
    for(int i = 0; i < SIZEOFTHING; i++) {
        mem[i] = 'j';
    }
    pthread_join(thread, NULL);
    printf("Memory shared safely %c\n", mem[12]);
    free(mem);
    // not yet supported
    // pthread_create(&thread, 0, testret, NULL);
    // int* ret;
    // pthread_join(thread, (void**) &ret);
    // printf("Value: %d\n", *ret);
    // free(ret);
}