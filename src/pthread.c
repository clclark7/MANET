#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *print_hello(void *arg) {
    printf("Hello from thread %ld\n", pthread_self());
    return NULL;
}

int main() {
    pthread_t thread_id;
    
    // Create a new thread
    pthread_create(&thread_id, NULL, print_hello, NULL);
    
    // Wait for the thread to finish
    pthread_join(thread_id, NULL);
    
    printf("Main program finished.\n");
    return 0;
}
