#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {
    // Close file descriptor 1
    close(1);

    // Opens a file descriptor (since 1 is open, output file will be on 1)
    int fd = open("output.txt", O_RDWR|O_CREAT);
    printf("Hello World");

    // TODO: Change permissions of file

    exit(0);

    return 0;
}