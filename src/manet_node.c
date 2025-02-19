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

// HELLO Message Sender Thread
void *send_hello(void *arg) {
    int sockfd = *((int *)arg);
    struct sockaddr_in broadcast_addr;

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    int broadcast_enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

    while (1) {
        char hello_msg[] = "HELLO from node!";
        sendto(sockfd, hello_msg, strlen(hello_msg), 0,
               (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
        printf("[HELLO] Broadcast sent!\n");

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

    // ✅ Step 2: Bind to port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("[MANET NODE] Listening on port %d\n", PORT);

    // Start HELLO sender thread
    pthread_t hello_thread;
    pthread_create(&hello_thread, NULL, send_hello, (void *)&sockfd);
    pthread_detach(hello_thread);

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
