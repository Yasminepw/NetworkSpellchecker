#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include <pthread.h>

#define NUM_WORK 10

void *worker_thread(void *);
void *log_thread(void *);

typedef struct node {
    void *data;
    struct node *next;
} node;

typedef struct queue {
    node *head;
    node *tail;
    int len;
} queue;

queue *initQueue();
node *initNode(void *);
void addQueue(queue *, void *);
void *removeQueue(queue *);
int empty(queue *);

#endif 
