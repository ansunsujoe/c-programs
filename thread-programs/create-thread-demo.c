#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

volatile int counter = 0;
int times;

// The worker function that is executed inside each thread
void *worker(void *arg) {
    int i;
    // Increment the counter
    for (i = 0; i < times; i++) {
        counter++;
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    // Check if there is only 1 argument
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments");
        exit(1);
    }

    // Convert the input to integer
    times = atoi(argv[1]);

    pthread_t p1, p2;
    printf("Initial value of product: %d\n", counter);

    // Initialize the threads to execute the worker function
    pthread_create(&p1, NULL, worker, NULL);
    pthread_create(&p2, NULL, worker, NULL);

    // The tasks are completed and are waiting for each other
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    // This doubling works correctly for small values, but not for large values
    // due to concurrency issues
    printf("Final value of product (result of doubling): %d\n", counter);

    return 0;

}