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
#define NUM_WORK 10

typedef struct Node {
  struct sockaddr_in client_addr;
  int client_socket;
  char *word;
  struct Node *next;
}Node;

typedef struct Queue {
  Node *head;
  int qsize;
}Queue;

Queue *initQueue();
Node *initNode(struct sockaddr_in, char *, int);
void addQueue(Queue *, struct sockaddr_in , char *, int);
void *removeQueue(Queue *);
int empty(Queue *);
void *worker_thread(void *);
void *log_thread(void *);

#endif //SERVER_H
