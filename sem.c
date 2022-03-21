#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "sem.h"

// gcc -pthread -o sem sem.c

typedef struct arg_struct
{
    char id[2];
    SEM *semaphore;
} arg_struct;

pthread_t tid[2];
int counter;

void P(SEM *sem)
{
    pthread_mutex_lock(&sem->lock);

    while(sem->counter <= 0) {
        pthread_cond_wait(&sem->condition, &sem->lock);
    }
    sem->counter--;

    pthread_mutex_unlock(&sem->lock);
}

void V(SEM *sem)
{
    pthread_mutex_lock(&sem->lock);

    printf("Sending signal\n");
    pthread_cond_signal(&sem->condition);
    sem->counter++;

    pthread_mutex_unlock(&sem->lock);
}

SEM *sem_init(int initVal)
{
    SEM *semaphore = malloc(sizeof(struct SEM));
    semaphore->counter = initVal;
    if (pthread_mutex_init(&semaphore->lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        sem_del(&semaphore);
        free(semaphore);
        return NULL;
    }
    if (pthread_cond_init(&semaphore->condition, NULL) != 0)
    {
        printf("\n condition init failed\n");
        free(semaphore);
        return NULL;
    }

    return semaphore;
}

int sem_del(SEM *sem)
{
    int return_val = 0;

    if (pthread_mutex_destroy(&sem->lock) != 0) {
        return_val = -1;
    }
    if (pthread_cond_destroy(&sem->condition) != 0) {
        return_val = -1;
    }

    free(sem);
    return return_val;
}

void *doSomething(void *args)
{
    arg_struct *arguments;
    arguments = (arg_struct *)args;
    P(arguments->semaphore);
    printf("Start, id: %s \n", arguments->id);
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