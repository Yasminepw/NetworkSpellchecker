#include "server.h"

//Creation of the queue 
Queue *initQueue() {

    Queue *temp = (Queue *) malloc(sizeof(Queue));

    if(temp == NULL) {
        printf("Queue memory allocation error!\n");
        exit(1);
    }
    temp->head = NULL;
    temp->qsize = 0;
    return temp;
}

void removeQueue(Queue *queue) {

    free(queue->head);
    free(queue);
}

// Creation of Node 
Node *initNode(struct sockaddr_in client, char *word, int socket) {

    Node *temp = (Node *) malloc(sizeof(Node));
    if(temp == NULL) {
        printf("Node memory allocation error!\n");
        exit(1);
     }
     temp->client_addr = client;
    if(word == NULL) {
        temp->word = word;
     } else {
        temp->word = malloc(sizeof(char *) * strlen(word) + 1);
    if(temp->word == NULL) {
        printf("Unable to allocate memory for Node.\n");
         exit(1);
    }
    strcpy(temp->word, word);
    }
    temp->next = NULL;
    temp->client_socket = socket;
    return temp;
}

// Pushes node onto the queue 
void push(Queue *queue, struct sockaddr_in client, char *word, int socket) {

    Node *temp = create_node(client, word, socket);

    if(queue->qsize == 0) {
        queue->head = temp;
    } else {
        Node * head = queue->head;
        while(head->next != NULL) {
            head = head->next;
        }
    head->next = temp;
    }
    queue->qsize++;
}

//Pops the first Node struct off of the queue 
Node *pop(Queue *queue) {
 
    if (queue->head == NULL) {
        queue->qsize = 0;
        return NULL;
    }

    //Replaces first Node and returns it 
    Node *temp = queue->head;
    queue->head = queue->head->next;
    queue->qsize--;
    free(queue->head);
    return temp;
}