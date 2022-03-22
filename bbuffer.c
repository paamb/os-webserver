#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "bbuffer.h"
#include "sem.h"

//Struct for testing:
typedef struct arg_struct
{
    BNDBUF *bbuffer;
    int write_value;
} arg_struct;

void bb_del(BNDBUF *bb){
    free(bb);
}

int bb_get(BNDBUF *bb){
    P(bb->read_sem);
    // printf("bb->read_sem: %p, bb->write_sem: %p\n", bb->read_sem, bb->write_sem);
    printf("READ bb->arrpointer: %p, bb->read_index: %d, bb->bb_size: %d, mod_read: %d, mod_write: %d\n", bb->arr_pointer, bb->read_index, bb->bb_size, ((bb->read_index) % bb->bb_size), ((bb->write_index) % bb->bb_size));
    // bb->read_index = ((bb->read_index + 1) % bb->bb_size);
    
    int read_val = bb->arr_pointer[((bb->read_index++) % bb->bb_size)];
    V(bb->write_sem);
    return read_val;
}

void bb_add(BNDBUF *bb, int fd){
    P(bb->write_sem);
    printf("WRITE bb->arrpointer: %p, bb->read_index: %d, bb->bb_size: %d, mod_read: %d, mod_write: %d\n", bb->arr_pointer, bb->read_index, bb->bb_size, ((bb->read_index) % bb->bb_size), ((bb->write_index) % bb->bb_size));
    // printf("Writing value \n");
    bb->write_index = ((bb->write_index + 1) % bb->bb_size);
    printf("WRITE2 bb->arrpointer: %p, bb->read_index: %d, bb->bb_size: %d, mod_read: %d, mod_write: %d\n", bb->arr_pointer, bb->read_index, bb->bb_size, ((bb->read_index) % bb->bb_size), ((bb->write_index) % bb->bb_size));
    // printf("write_index: %i \n", bb->write_index);
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

void *read_value(void *args)
{
    BNDBUF *buffer;
    buffer = (BNDBUF *)args;
    for (int i = 0; i < buffer->bb_size; i ++){
    printf("Read from buffer: %i \n", bb_get(buffer));
    }
    // return NULL;
}

void *write_value(void *args){
    arg_struct *arguments;
    arguments = (arg_struct *)args;
    for(int i= 0; i < 6; i++){
        for (int i = 0; i < 0xFFFFFFF; i++)
        ;
        bb_add(arguments->bbuffer, i);
        printf("Write to buffer: %i \n", i);
    }
    // bb_add(arguments->bbuffer, 60);
    // bb_add(arguments->bbuffer, 60);
    // bb_add(arguments->bbuffer, 60);

    // bb_add(arguments->bbuffer, 60);
    for (int i = 0; i < 0xFFFFFFF; i++)
        ;

    printf("Done writing \n");
    // return NULL;
}


// int main(void) {
//     BNDBUF *bbuffer = bb_init(3);
//     pthread_t thread_id_one, thread_id_two, thread_id_three;

//     arg_struct write_first = {.bbuffer = bbuffer, .write_value = 69};

    
//     pthread_create(&thread_id_one, NULL, read_value, (void *)bbuffer);
//     for (int i = 0; i < 0xFFFFFFF; i++)
//         ;
//     printf("bfefe \n");
//     pthread_create(&thread_id_two, NULL, write_value, (void *)&write_first);
//     for (int i = 0; i < 0xFFFFFFF; i++);

//     pthread_create(&thread_id_one, NULL, read_value, (void *)bbuffer);
//     printf("before waiting \n");
//     pthread_join(thread_id_two, NULL);
//     printf("after waiting \n");
//     pthread_join(thread_id_one, NULL);

//     //bb_del(bbuffer);
// }