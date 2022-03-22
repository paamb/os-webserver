#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "sem.h"

// gcc -pthread -o sem sem.c

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
        sem_del(semaphore);
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
