#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

int sockfd;

struct client_data {
    struct sockaddr_in addr;
    char message[BUFFER_SIZE];
};



void *handle_client(void *arg) {
    struct client_data *client_info = (struct client_data *)arg;
    socklen_t addr_len = sizeof(client_info->addr);

    printf("[THREAD %ld] Handling client %s\n", pthread_self(), inet_ntoa(client_info->addr.sin_addr));
    printf("[THREAD %ld] Message: %s\n", pthread_self(), client_info->message);

    // ✅ Force the thread to stay alive for 3 seconds
    struct timespec ts = {3, 0};
    nanosleep(&ts, NULL);

    // ✅ Send acknowledgment back to client
    char response[] = "Message received";
    sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_info->addr, addr_len);

    free(client_info);
    pthread_exit(NULL);
}



void initialize_socket() {
    struct sockaddr_in server_addr;

    // ✅ Step 1: Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // ✅ Step 2: Define server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(PORT); 

    // ✅ Step 3: Bind the socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        struct sockaddr_in *client_addr = malloc(sizeof(struct sockaddr_in));
        if (!client_addr) {
            perror("malloc failed");
            continue;
        }

        socklen_t addr_len = sizeof(*client_addr);
        pthread_t thread_id;
        char buffer[BUFFER_SIZE];

        // ✅ Step 4: Receive the message BEFORE creating the thread
        int bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)client_addr, &addr_len);
        if (bytes_received < 0) {
            perror("receive failed");
            free(client_addr);
            continue;
        }

        printf("[DEBUG] Received message from %s\n", inet_ntoa(client_addr->sin_addr));

        // ✅ Step 5: Allocate memory for `client_data`
        struct client_data *client_info = malloc(sizeof(struct client_data));
        if (!client_info) {
            perror("malloc failed");
            free(client_addr);
            continue;
        }

        // ✅ Copy client address and message
        client_info->addr = *client_addr;
        memcpy(client_info->message, buffer, bytes_received);
        client_info->message[bytes_received] = '\0';  // Ensure null termination

        free(client_addr);

        int ret = pthread_create(&thread_id, NULL, handle_client, (void *)client_info);
        if (ret != 0) {
            printf("[ERROR] pthread_create failed: %d\n", ret);
            free(client_info);
            continue;
        } else {
            printf("[DEBUG] Created new thread with ID: %ld\n", thread_id);
        }
        
        pthread_detach(thread_id);

    }
}


int main () {
    initialize_socket();
    return 0;
}
