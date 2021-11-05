#include "server.h"

queue *workQueue;
queue *logQueue;

pthread_mutex_t work_mutex, log_mutex;
pthread_cond_t cond1, cond2;

int main(int argc, char *argv[]) {

    workQueue = initQueue();
    logQueue = initQueue();

    // Delcare thread pool for workers and thread for logging
    pthread_t workers[NUM_WORK];
    pthread_t logger;

    // Initializing mutex and condition variables
    pthread_mutex_init(&work_mutex, NULL);
    pthread_cond_init(&cond1, NULL);
    pthread_mutex_init(&log_mutex, NULL);
    pthread_cond_init(&cond2, NULL);

    // Creation of log thread
    pthread_create(&logger, NULL, log_thread, NULL);

    // Declare socket variables and creation of socket
    int socketfd;
    struct sockaddr_in server_addr;
    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
        printf("Socket opening error.\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    // Binding socket
    if (bind(socketfd, (struct socketaddress *) &server_addr,
             sizeof(server_addr)) < 0) {
        printf("ERROR on binding\n");
        exit(1);
    }

    // Listening for socket
    listen(socketfd,5);

        // Access to connections queue
        pthread_mutex_lock(&work_mutex);
        // Add the new connection to the queue
        addQueue(workQueue, socketfd);
        // Tell workers of available connection
        pthread_cond_signal(&cond1);
        // Release mutual exclusion
        pthread_mutex_unlock(&work_mutex);

    return 0; 

}

void *worker_thread(void *arg){
    while(1) {
      printf("Socket = %d Thread = %ld Buffer = %d\n", get_socket(), pthread_self(), *workQueue);
    }
    return 0;
}

void *log_thread(void *arg) {
    FILE *log = fopen("log.txt", "r");

    while (1) {
        pthread_mutex_lock(&log_mutex);
        while (empty(logQueue))
            pthread_cond_wait(&cond2, &log_mutex);
    
        char *msg = removeQueue(logQueue);
        pthread_mutex_unlock(&log_mutex);
        fprintf(log, "%s", msg);
        fflush(log);
        free(msg);
    }


}
