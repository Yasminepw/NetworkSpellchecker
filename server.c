#include "server.h"

Queue *workQueue; 
Queue *logQueue; 
pthread_mutex_t work_lock, log_lock, logQueue_lock;
pthread_cond_t cond1, cond2, cond3, cond4; 

char** dict_words; //Holds all words found in dictionary

char ** dictionary(char *filename) {

    FILE *file = fopen(filename, "r"); //Opens file in read only 

    if(file == NULL) {
        printf("Error opening dictionary file!\n");
        exit(1);
    }

    char ** output = malloc(DICTIONARY_LENGTH * sizeof(char *) + 2);

    if(output == NULL) {
        printf("Memory allocation error.\n");
        exit(1);
     }

    char line [bufferSize];
    int index = 0; 

    //Copies the dictionary file line by line 
    while((fgets(line, bufferSize, file)) != NULL) {
        output[index] = (char *) malloc(strlen(line) * sizeof(char *) + 1);

        if(output[index] == NULL) {
            printf("Memory allocation error.\n");
            exit(1);
    }

    int temp = strlen(line) - 2;
    line[temp] = '\0';
    strcpy(output[index], line);
    index++;
  }

  fclose(file);
  return output;

  }


void *worker_thread(void *arg) {
    while(1) {
      printf("Socket = %d Thread = %ld Buffer = %d\n", get_socket(), pthread_self(), *workQueue);
    }
    return 0;
}


void *log_thread(void *arg) {
    FILE *log = fopen("log.txt", "r");

    while (1) {
        pthread_mutex_lock(&log_lock);
        while (empty(logQueue))
            pthread_cond_wait(&cond2, &log_lock);
    
        char *msg = removeQueue(logQueue);
        pthread_mutex_unlock(&log_lock);
        fprintf(log, "%s", msg);
        fflush(log);
        free(msg);
    }

}

int main(int argc, char *argv[]) {
    int port = 8889; //Default port  
    char *dict; 
    dict_words = dictionary(dict); //Gets the words found in the dictionary 

    pthread_mutex_init(&work_lock, NULL);
    pthread_mutex_init(&log_lock, NULL);
    pthread_mutex_init(&logQueue_lock, NULL);
    pthread_cond_init(&cond1, NULL);
    pthread_cond_init(&cond2, NULL);
    pthread_cond_init(&cond3, NULL);
    pthread_cond_init(&cond4, NULL);

     // Delcare thread pool for workers and thread for logging
    pthread_t workers[NUM_WORK];
    for (int i = 0; i < NUM_WORK; i++) {
        pthread_create(&workers[i], NULL, &worker_thread, NULL);
    }
    pthread_t logger;
    pthread_create(&logger, NULL, &log_thread, NULL);
    
    //Determines which port and dictionary to use 
    if(argc == 1) {
        port;  
        dict = DEFAULT_DICTIONARY;
    } else if(argc == 2) {
        port = atoi(argv[1]);
        dict = DEFAULT_DICTIONARY;
    } else {
        port = atoi(argv[1]);
        dict = argv[2];
    }

     //Socket creation and declation of variables 
    int socketfd; 
    char *message = "You are now connected to server. Please enter -1 to exit.\n";
    char *fullBuff = "The job buffer is full!\n";

    socketfd = socket(AF_INET, SOCK_STREAM, 0); 

    if(socketfd == -1){ 
        puts("Could not create socket."); 
        return 1; 
    }

    struct sockaddr_in address; 
    address.sin_family = AF_INET; 
    address.sin_port = htons(port); 
    address.sin_addr.s_addr = INADDR_ANY; //127.0.0.1

    bind(socketfd, (struct sockaddr *) &address, sizeof(address)); 
    puts("Spellcheck server started..."); 
    listen(socketfd, MAX_NET_BACKLOG); 

    int client_socket; 

    while (1) {
        client_socket = accept(socketfd, NULL, NULL); 

        if(client_socket < 0) { 
            puts("Unable to connect to client!"); 
        }

        int result; 
        result = send(client_socket, message, strlen(message), 0); 

        if(result == -1) { 
            puts("Error sending message!"); 
        }

        // Checks if full and locks work queue 
        pthread_mutex_lock(&work_lock);

        if(workQueue->qsize >= sizeMax) {
            send(client_socket, fullBuff, strlen(fullBuff), 0);
            pthread_cond_wait(&cond2, &work_lock);
        }

        push(workQueue, address, NULL, client_socket);
        pthread_mutex_unlock(&work_lock);
        pthread_cond_signal(&cond1);
    
    }  

    close(socketfd);
    return 0; 
} 

