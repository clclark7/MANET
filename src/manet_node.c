#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int sockfd;

// BROADCAST Message Sender Thread
void *broadcast(void *arg) {
    int sockfd = *((int *)arg);
    struct sockaddr_in broadcast_addr;

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    int broadcast_enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

    while (1) {
        char broadcast_msg[] = "broadcast from node!";
        sendto(sockfd, broadcast_msg, strlen(broadcast_msg), 0,
               (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
        printf("[BROADCAST] Broadcast sent!\n");

        sleep(5);
    }
    return NULL;
}

int main() {
    struct sockaddr_in server_addr, client_addr;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind to port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("[MANET NODE] Listening on port %d\n", PORT);

    // Start BROADCAST sender thread
    pthread_t broadcast_thread;
    pthread_create(&broadcast_thread, NULL, broadcast, (void *)&sockfd);
    pthread_detach(broadcast_thread);

    //Listen for incoming messages
    while (1) {
        char buffer[BUFFER_SIZE];
        socklen_t len = sizeof(client_addr);
        int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, 
                         (struct sockaddr *)&client_addr, &len);
        if (n < 0) {
            perror("Receive failed");
            exit(EXIT_FAILURE);
        }

        buffer[n] = '\0';
        printf("[RECEIVED] From %s: %s\n", inet_ntoa(client_addr.sin_addr), buffer);
    }

    close(sockfd);
    return 0;
}
