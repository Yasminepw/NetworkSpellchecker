//Yasmine Watkins
//CIS 3207 - 001
//Networked Spell checker

#include "server.h"

// Global buffer variables.
socketBuff socket_buff;
logBuff log_buff;

int sizeMax = 5; 
int logMax = 100; 
char **dictWords; 

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

void put(int client_socket) { 
    //This locks to work queue
    pthread_mutex_lock(&(socket_buff.work_lock));
    while(socket_buff.array_count >= sizeMax) {
        pthread_cond_wait(&socket_buff.cond_empty, &socket_buff.work_lock);
    }
        
    //Gets the first job off the queue 
    socket_buff.array[socket_buff.fill_ptr] = client_socket; 
    socket_buff.fill_ptr = (socket_buff.fill_ptr + 1) % sizeMax; 
    socket_buff.array_count++; 
    pthread_cond_signal(&socket_buff.cond_full);
    pthread_mutex_unlock(&socket_buff.work_lock);

}

int get() { 
    //This locks to work queue
    pthread_mutex_lock(&(socket_buff.work_lock));
    while(socket_buff.array_count < 1) {
        pthread_cond_wait(&socket_buff.cond_full, &socket_buff.work_lock);
    }
    //Gets the first job off the queue 
    int client = socket_buff.array[socket_buff.use_ptr]; 
    socket_buff.use_ptr = (socket_buff.use_ptr + 1) % sizeMax; 
    socket_buff.array_count--; 
    pthread_cond_signal(&socket_buff.cond_empty);
    pthread_mutex_unlock(&socket_buff.work_lock);
    return client;

}

void* worker_thread(void* args) {
    char *promptMsg = "Enter word to spellcheck: ";
    char *closeMsg = "Connection to server is terminated!\n";
    char word_recv[bufferSize];
    int client_socket; 

    while (1) { 
        client_socket = get();
        send(client_socket, promptMsg, strlen(promptMsg), 0);

        while(read(client_socket, word_recv, bufferSize) > 0) { 
            //Terminates connection of user enters -1 
            if (atoi(&word_recv[0]) == -1) {
                send(client_socket, closeMsg, strlen(closeMsg), 0);
                close(client_socket);
                return 0;
            } else {
        
                word_recv[strlen(word_recv) - 1] = '\0'; 
                //Word is mispelled
                char *result = " MISSPELLED\n";
                for (int i = 0; i < DICTIONARY_LENGTH; i++) {
                    //Word is spelled correctly
                    if (strcmp(word_recv, dictWords[i]) == 0) {
                        result = " OK\n";
                        break;
                    }
                }

                strcat(word_recv, result);
                //printf("%s", word_recv);
                send(client_socket, word_recv, strlen(word_recv), 0);
                //Puts record of words in logfile
                putLog(word_recv); 
            }
            memset(word_recv, 0, bufferSize);
        }

    }
    return 0; 
} 

void putLog(char *wordLog) {
    //This locks to work queue
    pthread_mutex_lock(&(log_buff.log_lock));
    while(log_buff.array_count >= logMax) {
        pthread_cond_wait(&log_buff.cond_empty, &log_buff.log_lock);
    }
        
    //Gets the first job off the queue 
    log_buff.array[log_buff.fill_ptr] = wordLog; 
    log_buff.fill_ptr = (log_buff.fill_ptr + 1) % logMax; 
    log_buff.array_count++; 
    pthread_cond_signal(&log_buff.cond_full);
    pthread_mutex_unlock(&log_buff.log_lock);
} 

char * getLog() { 
    //This locks to work queue
    pthread_mutex_lock(&(log_buff.log_lock));
    while(log_buff.array_count < 1) {
        pthread_cond_wait(&log_buff.cond_full, &log_buff.log_lock);
    } 
    //Gets the first job off the queue 
    char *word = log_buff.array[log_buff.use_ptr]; 
    log_buff.use_ptr = (log_buff.use_ptr + 1) % logMax; 
    log_buff.array_count--; 
    pthread_cond_signal(&log_buff.cond_empty);
    pthread_mutex_unlock(&log_buff.log_lock);
    return word; 
}

void *log_thread(void *arg) {
    char *word;
    FILE *log_file;
     while(1) {
        log_file = fopen(DEFAULT_LOG_FILE, "a");
        word = getLog(); 

        // Write results to log file. 
       
        fprintf(log_file, "%s\n", word);
        //fprintf(stdout,"%s\n", word); 
        fclose(log_file);
    }
}

int main(int argc, char **argv) { 
    char *dict; 
    int port; 
    socket_buff.array = malloc(sizeMax * sizeof(int));
    socket_buff.fill_ptr = 0; 
    socket_buff.use_ptr = 0; 
    socket_buff.array_count = 0; 
    log_buff.array = malloc(logMax * sizeof(char));
    log_buff.fill_ptr = 0; 
    log_buff.use_ptr = 0; 
    log_buff.array_count = 0; 


    //Determines which port and dictionary to use 
    if(argc == 1){ 
        port = DEFAULT_PORT; 
        dict = DEFAULT_DICTIONARY; 

    } else if(argc == 2){ 
        port = atoi(argv[1]); 
        dict = DEFAULT_DICTIONARY; 
    } else { 
        port = atoi(argv[1]); 
        dict = argv[2]; 

    }

    //Gets the dictionary words.
    dictWords = dictionary(dict);
   
    pthread_mutex_init(&socket_buff.work_lock, NULL);
    pthread_mutex_init(&log_buff.log_lock, NULL);
    pthread_cond_init(&socket_buff.cond_full, NULL);
    pthread_cond_init(&socket_buff.cond_empty, NULL);
    pthread_cond_init(&log_buff.cond_full, NULL);
    pthread_cond_init(&log_buff.cond_empty, NULL);

    // Delcare thread pool for workers and thread for logging
    pthread_t logger;
    pthread_create(&logger, NULL, &log_thread, NULL);
    pthread_t workers[NUM_WORK];
    
    for (int i = 0; i < NUM_WORK; i++) {
        pthread_create(&workers[i], NULL, &worker_thread, NULL);
    }

     //Socket creation and declation of variables 
    int socketfd; 
    char *message = "You are now connected to server. Please enter -1 to exit.\n";
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

        put(client_socket);
        
    }  

    close(socketfd);
    return 0; 
}