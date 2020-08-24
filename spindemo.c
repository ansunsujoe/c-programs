#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

int main(int argc, char *argv[]) {

    // Argc equals 2 means that there is 1 argument on execution
    if (argc != 2) {
        fprintf(stderr, "Not correct amount of arguments");
        // Exit with error
        exit(1);
    }

    // Take in the first argument
    char *str = argv[1];

    // Loop forever
    while (1) {
        sleep(1);   // Sleep for one second
        printf("%s >\n", str);
    }

    return 0;

}