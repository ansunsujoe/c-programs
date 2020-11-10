#include <stdio.h>
#include <string.h>

/**
 * This is just a playground
 */
int main() {

    int x = 10;
    int rc = fork();

    if (rc == 0) {
        printf("Child: pid = %d\n", getpid());
    }
    else {
        wait();
        printf("Parent: pid = %d\n", getpid());
    }

    return 0;
}