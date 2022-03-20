#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "sem.h"

typedef struct arg_struct
{
    char id[2];
    SEM *semaphore;
} arg_struct;

pthread_t tid[2];
int counter;

void P(SEM *sem)
{
    if (sem->counter == 1)
    {
        pthread_mutex_lock(&sem->lock);
    }
    else
    {
        sem->counter--;
    }
}
void V(SEM *sem)
{
    pthread_mutex_unlock(&sem->lock);
    sem->counter++;
}

SEM *sem_init(int initVal)
{
    static SEM semaphore;
    semaphore.counter = initVal;
    if (pthread_mutex_init(&semaphore.lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        sem_del(&semaphore);
        return NULL;
    }
    return &semaphore;
}

int sem_del(SEM *sem)
{
    if (pthread_mutex_destroy(&sem->lock) == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void *doSomething(void *args)
{
    arg_struct *arguments;
    arguments = (arg_struct *)args;
    P(arguments->semaphore);
    printf("Start, id: %s \n", arguments->id);
    for (int i = 0; i < 0xFFFFFFF; i++)
        ;
    for (int i = 0; i < 0xFFFFFFF; i++)
        ;
    printf("Finished :D, id: %s \n", arguments->id);
    V(arguments->semaphore);

    return NULL;
}

int main(void)
{
    SEM *semaphore = sem_init(2);
    pthread_t thread_id_one, thread_id_two, thread_id_three;

    arg_struct argsone = {.id = "1", .semaphore = semaphore};
    arg_struct argstwo = {.id = "2", .semaphore = semaphore};
    arg_struct argsthree = {.id = "3", .semaphore = semaphore};

    pthread_create(&thread_id_one, NULL, doSomething, (void *)&argsone);
    pthread_create(&thread_id_two, NULL, doSomething, (void *)&argstwo);
    pthread_create(&thread_id_three, NULL, doSomething, (void *)&argsthree);

    pthread_join(thread_id_one, NULL);
    pthread_join(thread_id_two, NULL);
    pthread_join(thread_id_three, NULL);

    sem_del(semaphore);

    return 0;
}