#include "server.h"

// Global buffer variables.
socketBuff socket_buff;
logBuff log_buff;

int sizeMax = 5; 
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
    socket_buff.fill_ptr = (socket_buff.fill_ptr + 1) % socket_buff.array_count; 
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
    socket_buff.use_ptr = (socket_buff.use_ptr + 1) % socket_buff.array_count; 
    socket_buff.array_count--; 
    pthread_cond_signal(&socket_buff.cond_empty);
    pthread_mutex_unlock(&socket_buff.work_lock);
    return client;

}

void* worker_thread(void* args) {
    char *promptMsg = "Enter word to spellcheck: ";
    char *closeMsg = "Connection to server is terminated!\n";
    char *errorMsg = "Error displaying message!\n";

    int client_socket = get();

    while (1) { 
        char buff_recv[bufferSize] = "";
        //This sends the prompt to the buffer
        send(client_socket, promptMsg, strlen(promptMsg), 0);
        int returnedWord = recv(client_socket, buff_recv, bufferSize, 0);
        //Error check
        if (returnedWord < 0) {
            send(client_socket, errorMsg, strlen(errorMsg), 0);
            continue;
        //Quitting client
        } else if (atoi(&buff_recv[0]) == -1) {
            send(client_socket, closeMsg, strlen(closeMsg), 0);
            close(client_socket);
            break;
        //Checking if word is found in dictionary
        } else {
            buff_recv[strlen(buff_recv) - 1] = '\0';
            buff_recv[returnedWord - 2] = '\0';
        
        //Word is mispelled
        char *result = " MISSPELLED\n";
        for (int i = 0; i < DICTIONARY_LENGTH; i++) {
            //Word is spelled correctly
            if (strcmp(buff_recv, dictWords[i]) == 0) {
                result = " OK\n";
                break;
            }
        }

        strcat(buff_recv, result);
        printf("%s", buff_recv);
        send(client_socket, buff_recv, strlen(buff_recv), 0);

        // //Locking log 
        // pthread_mutex_lock(&logQueue_lock);

        // if(logQueue->qsize >= sizeMax) {
        //     pthread_cond_wait(&empty, &logQueue_lock);

        // }
        // // Adds result to log buffer
        // push(logQueue, client, buff_recv, client_socket);
        // pthread_mutex_unlock(&logQueue_lock);
        // pthread_cond_signal(&full);

        }
    }
    return 0; 
} 

// void *log_thread(void *arg) {

//      while(1) {
//         /* Locks the log queue. */
//         pthread_mutex_lock(&logQueue_lock);
//         if (logQueue->qsize == 0) {
//             /* Waits if empty. */
//             pthread_cond_wait(&full, &logQueue_lock);
//         }
//         /* Gets the word. */
//         Node *node = pop(logQueue);
//         char *word = node->word;

//         /* Releases the lock. */
//         pthread_mutex_unlock(&logQueue_lock);
//         /* Sends the signal. */
//         pthread_cond_signal(&empty);

//         /* If empty do nothing. */
//         if (word == NULL) {
//             continue;
//         }

//         /* Lock the log file. */
//         pthread_mutex_lock(&log_lock);

//         /* Write results to log file. */
//         FILE *log_file = fopen(DEFAULT_LOG_FILE, "a");
//         fprintf(log_file, "%s", word);
//         fclose(log_file);

//         /* Releases the lock. */
//         pthread_mutex_unlock(&log_lock);
//     }
// }

int main(int argc, char **argv) { 
    char *dict; 
    int port; 

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
   
    pthread_mutex_init(&socket_buff.work_lock, NULL);
    pthread_mutex_init(&log_buff.log_lock, NULL);
    pthread_cond_init(&socket_buff.cond_full, NULL);
    pthread_cond_init(&socket_buff.cond_empty, NULL);
    //pthread_cond_init(log_buff. , NULL);
    //pthread_cond_init(log_buff.cond_empty, NULL);

    // Delcare thread pool for workers and thread for logging
    pthread_t workers[NUM_WORK];
    
    for (int i = 0; i < NUM_WORK; i++) {
        pthread_create(&workers[i], NULL, &worker_thread, NULL);
    }
  
    //pthread_t logger;
    //pthread_create(&logger, NULL, &log_thread, NULL);

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