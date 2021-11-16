#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEFAULT_DICTIONARY "words.txt"
#define DEFAULT_PORT 8889
#define DEFAULT_LOG_FILE "log.txt"
#define MAX_NET_BACKLOG 1024
#define DICTIONARY_LENGTH 466474
#define bufferSize 256
#define NUM_WORK 5


typedef struct socketBuff {
    int *array;
    int fill_ptr;
    int use_ptr;
    int array_count;
    pthread_mutex_t work_lock;
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
}socketBuff;

typedef struct logBuff {
    char **array;
    int fill_ptr;
    int use_ptr;
    int array_count;
    pthread_mutex_t log_lock;
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
}logBuff;

void *worker_thread(void *);
void *log_thread(void *);

#endif //SERVER_H
