#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "bbuffer.h"
#include "sem.h"

void bb_del(BNDBUF *bb){
    free(bb);
}

int bb_get(BNDBUF *bb){
    P(bb->read_sem);
    int read_val = bb->arr_pointer[bb->read_index];
    bb->read_index = ((bb->read_index + 1) % bb->bb_size);
    V(bb->write_sem);
    return read_val;
}

void bb_add(BNDBUF *bb, int fd){
    P(bb->write_sem);
    bb->write_index = ((bb->write_index + 1) % bb->bb_size);
    bb->arr_pointer[bb->write_index] = fd;
    V(bb->read_sem);
}


BNDBUF *bb_init(unsigned int size){
    BNDBUF *buffer = malloc(sizeof(struct BNDBUF));
    SEM *write_sem = sem_init(size);
    SEM *read_sem = sem_init(0);
    buffer->bb_size = size;
    buffer->arr_pointer = (int*) malloc(size * sizeof(int));
    buffer->write_index = -1;
    buffer->read_index = 0;
    buffer->write_sem = write_sem;
    buffer->read_sem = read_sem;
    return buffer;
}
